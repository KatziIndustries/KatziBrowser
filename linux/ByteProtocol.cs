using System;
using System.Buffers.Binary;
using System.Net.Sockets;
using System.Threading;
using System.Threading.Tasks;

public static class ByteProtocol
{
    public const int HeaderSize = 8;
    public const uint MaxPayloadSize = 16 * 1024 * 1024;

    public static async Task SendMsgAsync(
        Socket socket,
        uint type,
        ReadOnlyMemory<byte> data,
        CancellationToken ct = default)
    {
        if (data.Length > MaxPayloadSize)
            throw new ArgumentOutOfRangeException(nameof(data));

        byte[] header = new byte[HeaderSize];

        BinaryPrimitives.WriteUInt32LittleEndian(
            header.AsSpan(0, 4), type);

        BinaryPrimitives.WriteUInt32LittleEndian(
            header.AsSpan(4, 4), (uint)data.Length);

        await SendAllAsync(socket, header, ct);

        if (!data.IsEmpty)
            await SendAllAsync(socket, data, ct);
    }

    private static async Task SendAllAsync(
        Socket socket,
        ReadOnlyMemory<byte> data,
        CancellationToken ct)
    {
        while (!data.IsEmpty)
        {
            int sent = await socket.SendAsync(
                data, SocketFlags.None, ct);

            if (sent == 0)
                throw new SocketException();

            data = data[sent..];
        }
    }

    public static async Task<bool> ReceiveMsgAsync(
        Socket socket,
        Func<uint, byte[], Task> onMessage,
        CancellationToken ct = default)
    {
        byte[] header = new byte[HeaderSize];

        if (!await ReceiveExactlyAsync(
                socket, header, allowInitialEof: true, ct))
            return false;

        uint type = BinaryPrimitives.ReadUInt32LittleEndian(
            header.AsSpan(0, 4));

        uint size = BinaryPrimitives.ReadUInt32LittleEndian(
            header.AsSpan(4, 4));

        if (size > MaxPayloadSize)
            throw new InvalidOperationException(
                $"Payload too large: {size}");

        byte[] payload = new byte[(int)size];

        if (size > 0)
            await ReceiveExactlyAsync(
                socket, payload, allowInitialEof: false, ct);

        await onMessage(type, payload);
        return true;
    }

    private static async Task<bool> ReceiveExactlyAsync(
        Socket socket,
        Memory<byte> buffer,
        bool allowInitialEof,
        CancellationToken ct)
    {
        int offset = 0;

        while (offset < buffer.Length)
        {
            int received = await socket.ReceiveAsync(
                buffer[offset..], SocketFlags.None, ct);

            if (received == 0)
            {
                if (offset == 0 && allowInitialEof)
                    return false;

                throw new System.IO.EndOfStreamException(
                    "Connection closed in the middle of a message.");
            }

            offset += received;
        }

        return true;
    }
}
