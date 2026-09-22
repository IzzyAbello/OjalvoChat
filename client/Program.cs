using System.Net.Sockets;
using System.Text;
using System.Text.Json;

int serverPort = (args.Length > 0) ? int.Parse(args[0]) : 1234;
string ServerHost = (args.Length > 1) ? args[1] : "127.0.0.1";

Console.WriteLine($"Conectando a {ServerHost}:{serverPort}...");

using TcpClient client = new TcpClient();
await client.ConnectAsync(ServerHost, serverPort);
Console.WriteLine("Conectado.");

using NetworkStream stream = client.GetStream();
using StreamReader reader = new StreamReader(stream, Encoding.UTF8);
using StreamWriter writer = new StreamWriter(
    stream,
    new UTF8Encoding(false)
) { AutoFlush = true };

object consoleLock = new object();
using CancellationTokenSource cts = new CancellationTokenSource();

Console.CancelKeyPress += (_, e) =>
{
    e.Cancel = true;
    lock (consoleLock) Console.WriteLine("\n¡Hasta luego!");
    cts.Cancel();
};

Task receiveTask = Task.Run(async () =>
{
    try
    {
        string? line;
        while ((line = await reader.ReadLineAsync(cts.Token)) != null)
        {
            lock (consoleLock)
            {
                Console.WriteLine($"\n<<< {line}");
            }
        }
        lock (consoleLock)
        {
            Console.WriteLine("\nEl servidor cerró la conexión.");
        }
        cts.Cancel();
    }
    catch (OperationCanceledException) {}
});

try
{
    while (!cts.IsCancellationRequested)
    {
        Console.WriteLine("Escribe el tipo de mensaje:");
        string messageType = Console.ReadLine()?.Trim().ToLower() ?? "";

        string outgoingJson;
        bool disconnect = false;

        switch (messageType)
        {
            case "disconnect":
                Console.WriteLine("Desconectando...");
                outgoingJson = JsonSerializer.Serialize(
                    new { type = "DISCONNECT" }
                );
                disconnect = true;
                break;
            case "identify":
                Console.WriteLine("Escribe tu username:");
                string username = Console.ReadLine() ?? "";
                outgoingJson = JsonSerializer.Serialize(
                    new { type = "IDENTIFY", username }
                );
                break;
            case "status":
                Console.WriteLine("Escribe tu status:");
                string status = Console.ReadLine() ?? "";
                outgoingJson = JsonSerializer.Serialize(
                    new { type = "STATUS", status }
                );
                break;
            case "users":
                outgoingJson = JsonSerializer.Serialize(
                    new { type = "USERS" }
                );
                break;
            case "text":
                Console.WriteLine("Escribe el destinatario:");
                string usernameText = Console.ReadLine() ?? "";
                Console.WriteLine("Escribe el texto:");
                string text = Console.ReadLine() ?? "";
                outgoingJson = JsonSerializer.Serialize(
                    new { type = "TEXT", username = usernameText, text }
                );
                break;
            case "public":
                Console.WriteLine("Escribe el texto público:");
                string publicText = Console.ReadLine() ?? "";
                outgoingJson = JsonSerializer.Serialize(
                    new { type = "PUBLIC_TEXT", text = publicText }
                );
                break;
            case "new":
                Console.WriteLine("Escribe el nombre del cuarto:");
                string roomname = Console.ReadLine() ?? "";
                outgoingJson = JsonSerializer.Serialize(
                    new { type = "NEW_ROOM", roomname }
                );
                break;
            case "join":
                Console.WriteLine("Escribe el nombre del cuarto:");
                string roomName = Console.ReadLine() ?? "";
                outgoingJson = JsonSerializer.Serialize(
                    new { type = "JOIN_ROOM", roomname = roomName }
                );
                break;
            case "room users":
                Console.WriteLine("Escribe el nombre del cuarto:");
                string room = Console.ReadLine() ?? "";
                outgoingJson = JsonSerializer.Serialize(
                    new { type = "ROOM_USERS", roomname = room }
                );
                break;
            case "room text":
                Console.WriteLine("Escribe el nombre del cuarto:");
                string roomNm = Console.ReadLine() ?? "";
                Console.WriteLine("Escribe el texto:");
                string roomTxt = Console.ReadLine() ?? "";
                outgoingJson = JsonSerializer.Serialize(
                    new { type = "ROOM_TEXT", roomname = roomNm, text = roomTxt }
                );
                break;
            case "leave room":
                Console.WriteLine("Escribe el nombre del cuarto:");
                string roomN = Console.ReadLine() ?? "";
                outgoingJson = JsonSerializer.Serialize(
                    new { type = "LEAVE_ROOM", roomname = roomN }
                );
                break;
            case "invite":
                Console.WriteLine("Escribe el nombre del cuarto:");
                string roomNam = Console.ReadLine() ?? "";
                Console.WriteLine("Escribe los nombres de los invitados separados por comas:");
                string usernamesRaw = Console.ReadLine() ?? "";
                string[] usernamesArray = usernamesRaw
                    .Split(',')
                    .Select(u => u.Trim())
                    .ToArray();
                
                outgoingJson = JsonSerializer.Serialize(
                    new 
                    {
                        type = "INVITE",
                        roomname = roomNam,
                        usernames = usernamesArray
                    }
                );
                break;
            default:
                Console.WriteLine("Tipo de mensaje inválido.");
                continue;
        }

        lock (consoleLock)
        {
            Console.WriteLine($">>> {outgoingJson}");
        }
        await writer.WriteLineAsync(outgoingJson);

        if (disconnect) break;
    }
}
finally
{
    cts.Cancel();
}

await receiveTask;