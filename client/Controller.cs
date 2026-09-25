using System.Net.Sockets;
using System.Text;
using System.Text.Json;

namespace ChatClient;

public class Controller
{
    private readonly View _view;
    private readonly string _host;
    private readonly int _port;

    private TcpClient _client = new();
    private StreamReader _reader = null!;
    private StreamWriter _writer = null!;
    private readonly object _consoleLock = new();

    public Controller(View view, string host, int port)
    {
        _view = view;
        _host = host;
        _port = port;
    }

    public async Task RunAsync()
    {
        _view.ShowConnecting(_host, _port);
        try
        {
            await _client.ConnectAsync(_host, _port);
        }
        catch (Exception ex)
        {
            _view.ShowConnectionError(ex);
            return;
        }
        _view.ShowConnected();

        NetworkStream stream = _client.GetStream();
        _reader = new StreamReader(stream, Encoding.UTF8);
        _writer = new StreamWriter(stream, new UTF8Encoding(false)) { AutoFlush = true, NewLine = "\n" };

        using CancellationTokenSource cts = new();

        Console.CancelKeyPress += (_, e) =>
        {
            e.Cancel = true;
            cts.Cancel();
        };

        Task receiveTask = ReceiveLoopAsync(cts);
        await SendLoopAsync(cts);
        await receiveTask;

        _view.ShowGoodbye();
    }

    private async Task ReceiveLoopAsync(CancellationTokenSource cts)
    {
        try
        {
            string? line;
            while ((line = await _reader.ReadLineAsync(cts.Token)) != null)
            {
                if (line.Length == 0) continue;

                ServerMessage? message = ServerMessage.Parse(line);

                lock (_consoleLock)
                {
                    if (message is null) _view.ShowInvalidMessage(line);
                    else message.Display(_view);
                }
            }
            lock (_consoleLock) _view.ShowServerClosed();
        }
        catch (OperationCanceledException) { }
        finally
        {
            cts.Cancel();
        }
    }

    private async Task SendLoopAsync(CancellationTokenSource cts)
    {
        try
        {
            while (!cts.IsCancellationRequested)
            {
                string messageType = (_view.AskMessageType() ?? "").Trim().ToLower();

                string outgoingJson;
                bool disconnect = false;

                switch (messageType)
                {
                    case "help":
                        _view.ShowHelp();
                        continue;
                    case "disconnect":
                        outgoingJson = JsonSerializer.Serialize(new { type = "DISCONNECT" });
                        disconnect = true;
                        break;
                    case "identify":
                        string username = _view.AskUsername() ?? "";
                        outgoingJson = JsonSerializer.Serialize(new { type = "IDENTIFY", username });
                        break;
                    case "status":
                        string status = _view.AskStatus() ?? "";
                        status = status.ToUpper();
                        outgoingJson = JsonSerializer.Serialize(new { type = "STATUS", status});
                        break;
                    case "users":
                        outgoingJson = JsonSerializer.Serialize(new { type = "USERS" });
                        break;
                    case "text":
                        string usernameText = _view.AskRecipient() ?? "";
                        string text = _view.AskText() ?? "";
                        outgoingJson = JsonSerializer.Serialize
                        (
                            new { type = "TEXT", username = usernameText, text }
                        );
                        break;
                    case "public":
                        string publicText = _view.AskPublicText() ?? "";
                        outgoingJson = JsonSerializer.Serialize
                        (
                            new { type = "PUBLIC_TEXT", text = publicText }
                        );
                        break;
                    case "new room":
                        string roomname = _view.AskRoomName() ?? "";
                        outgoingJson = JsonSerializer.Serialize(new { type = "NEW_ROOM", roomname });
                        break;
                    case "join":
                        string roomName = _view.AskRoomName() ?? "";
                        outgoingJson = JsonSerializer.Serialize
                        (
                            new { type = "JOIN_ROOM", roomname = roomName }
                        );
                        break;
                    case "room users":
                        string room = _view.AskRoomName() ?? "";
                        outgoingJson = JsonSerializer.Serialize
                        (
                            new { type = "ROOM_USERS", roomname = room }
                        );
                        break;
                    case "room text":
                        string roomNm = _view.AskRoomName() ?? "";
                        string roomTxt = _view.AskText() ?? "";
                        outgoingJson = JsonSerializer.Serialize
                        (
                            new { type = "ROOM_TEXT", roomname = roomNm, text = roomTxt }
                        );
                        break;
                    case "leave room":
                        string roomN = _view.AskRoomName() ?? "";
                        outgoingJson = JsonSerializer.Serialize
                        (
                            new { type = "LEAVE_ROOM", roomname = roomN }
                        );
                        break;
                    case "invite":
                        string roomNam = _view.AskRoomName() ?? "";
                        string usernamesRaw = _view.AskGuests() ?? "";
                        string[] usernamesArray = usernamesRaw
                            .Split(',')
                            .Select(u => u.Trim())
                            .Where(u => u.Length > 0)
                            .ToArray();
                        outgoingJson = JsonSerializer.Serialize(new { type = "INVITE", roomname = roomNam, usernames = usernamesArray });
                        break;
                    default:
                        _view.ShowInvalidCommand();
                        continue;
                }            

                lock (_consoleLock) _view.ShowOutgoing(messageType);
                await _writer.WriteLineAsync(outgoingJson);

                if (disconnect) break;
            }
        }
        catch (OperationCanceledException) { }
        finally
        {
            cts.Cancel();
        }
    }
}