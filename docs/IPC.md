# Interprocesscommunication

The main KatziBrowser process talks to the WebContent process
over a socket (/tmp/webview.sock)

## Specs
```txt
┌───────────────────────┬───────────────────────┐
│ type: uint32 (4 bytes)│ size: uint32 (4 bytes)│
├───────────────────────┴───────────────────────┤
│ payload: size bytes                           │
└───────────────────────────────────────────────┘
```

Every Package in the communication protocol used by Katzi
Browser has a type (uint32) a size (uint32) and the payload with the length of size.