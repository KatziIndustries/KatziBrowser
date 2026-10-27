
using System;
using System.Collections.Concurrent;
using System.IO;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Channels;
using System.Threading.Tasks;

public sealed record CommunicationMessage(uint Type, byte[] Payload);

public sealed class Communication : IAsyncDisposable
{
    private const string SocketPath = "/tmp/webview.sock";

    private readonly CancellationTokenSource cts = new();
    private readonly Socket server;

    private readonly ConcurrentDictionary<Socket, SemaphoreSlim> clients = new();

    private readonly Channel<CommunicationMessage> messages =
        Channel.CreateUnbounded<CommunicationMessage>(
            new UnboundedChannelOptions
            {
                SingleReader = false,
                SingleWriter = false
            });

    private readonly ConcurrentDictionary<Socket, Task> clientTasks = new();

    private Task? serverTask;
    private int disposed;

    public Communication()
    {
        if (File.Exists(SocketPath))
            File.Delete(SocketPath);

        server = new Socket(
            AddressFamily.Unix,
            SocketType.Stream,
            ProtocolType.Unspecified);

        server.Bind(new UnixDomainSocketEndPoint(SocketPath));
        server.Listen(128);
    }

    public Task InitAsync()
    {
        if (serverTask != null)
            throw new InvalidOperationException("Server already started.");

        Console.CancelKeyPress += OnCancelKeyPress;
        serverTask = CheckAsync(cts.Token);
        return serverTask;
    }

    private void OnCancelKeyPress(object? sender, ConsoleCancelEventArgs e)
    {
        e.Cancel = true;
        cts.Cancel();
    }

    private async Task CheckAsync(CancellationToken ct)
    {
        Console.WriteLine($"Listening on {SocketPath}");

        try
        {
            while (!ct.IsCancellationRequested)
            {
                Socket client;

                try
                {
                    client = await server.AcceptAsync(ct);
                }
                catch (OperationCanceledException)
                {
                    break;
                }
                catch (ObjectDisposedException) when (ct.IsCancellationRequested)
                {
                    break;
                }
                catch (SocketException) when (ct.IsCancellationRequested)
                {
                    break;
                }

                Console.WriteLine("Client connected");

                var sendLock = new SemaphoreSlim(1, 1);
                clients.TryAdd(client, sendLock);

                Task task = HandleClientAsync(client, sendLock, ct);
                clientTasks.TryAdd(client, task);

                _ = task.ContinueWith(
                    _ => clientTasks.TryRemove(client, out Task?_),
                    CancellationToken.None,
                    TaskContinuationOptions.ExecuteSynchronously,
                    TaskScheduler.Default);
            }
        }
        finally
        {
            Console.WriteLine("Server stopped accepting clients");
        }
    }

    private async Task HandleClientAsync(
        Socket client,
        SemaphoreSlim sendLock,
        CancellationToken ct)
    {
        try
        {
            while (await ByteProtocol.ReceiveMsgAsync(
                client,
                async (type, payload) =>
                {
                    var message = new CommunicationMessage(type, payload);

                    await messages.Writer.WriteAsync(message, ct);

                    Console.WriteLine(
                        $"Received Type={type}, Size={payload.Length}");

                    if (type == 1)
                    {
                        Console.WriteLine(
                            $"Text: {Encoding.UTF8.GetString(payload)}");
                    }
                },
                ct))
            {
            }
        }
        catch (OperationCanceledException)
        {
        }
        catch (SocketException ex)
        {
            Console.WriteLine($"Socket error: {ex.Message}");
        }
        catch (EndOfStreamException ex)
        {
            Console.WriteLine($"Truncated message: {ex.Message}");
        }
        catch (ObjectDisposedException)
        {
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Client error: {ex.Message}");
        }
        finally
        {
            clients.TryRemove(client, out _);

            try
            {
                client.Shutdown(SocketShutdown.Both);
            }
            catch (SocketException)
            {
            }
            catch (ObjectDisposedException)
            {
            }

            client.Dispose();

            await sendLock.WaitAsync();
            sendLock.Dispose();

            Console.WriteLine("Client disconnected");
        }
    }

    public async Task SendAsync(
        Socket client,
        uint type,
        ReadOnlyMemory<byte> payload,
        CancellationToken ct = default)
    {
        if (!clients.TryGetValue(client, out var sendLock))
            throw new InvalidOperationException("Client is not connected.");

        await sendLock.WaitAsync(ct);

        try
        {
            if (!clients.ContainsKey(client))
                throw new InvalidOperationException("Client disconnected.");

            await ByteProtocol.SendMsgAsync(client, type, payload, ct);
        }
        finally
        {
            sendLock.Release();
        }
    }

    public async Task BroadcastAsync(
        uint type,
        ReadOnlyMemory<byte> payload,
        CancellationToken ct = default)
    {
        foreach (var client in clients.Keys)
        {
            try
            {
                await SendAsync(client, type, payload, ct);
            }
            catch (SocketException)
            {
            }
            catch (ObjectDisposedException)
            {
            }
            catch (InvalidOperationException)
            {
            }
        }
    }

    public Task SendTextAsync(
        Socket client,
        string text,
        CancellationToken ct = default)
    {
        return SendAsync(
            client,
            1,
            Encoding.UTF8.GetBytes(text),
            ct);
    }

    public Task BroadcastTextAsync(
        uint type,
        string text,
        CancellationToken ct = default)
    {
        return BroadcastAsync(
            type,
            Encoding.UTF8.GetBytes(text),
            ct);
    }

    public bool TryPollMessage(out CommunicationMessage? message)
    {
        return messages.Reader.TryRead(out message);
    }

    public ValueTask<CommunicationMessage> ReadMessageAsync(
        CancellationToken ct = default)
    {
        return messages.Reader.ReadAsync(ct);
    }

    public int ConnectedClientCount => clients.Count;

    public async ValueTask DisposeAsync()
    {
        if (Interlocked.Exchange(ref disposed, 1) != 0)
            return;

        Console.CancelKeyPress -= OnCancelKeyPress;

        cts.Cancel();
        server.Dispose();

        foreach (var client in clients.Keys)
        {
            try
            {
                client.Dispose();
            }
            catch (ObjectDisposedException)
            {
            }
        }

        if (serverTask != null)
        {
            try
            {
                await serverTask;
            }
            catch (OperationCanceledException)
            {
            }
        }

        var tasks = clientTasks.Values;

        try
        {
            await Task.WhenAll(tasks);
        }
        catch (OperationCanceledException)
        {
        }

        messages.Writer.TryComplete();
        cts.Dispose();

        try
        {
            if (File.Exists(SocketPath))
                File.Delete(SocketPath);
        }
        catch (IOException ex)
        {
            Console.WriteLine($"Could not remove socket file: {ex.Message}");
        }
    }
}
