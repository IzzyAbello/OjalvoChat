using System.Net.Sockets;
using System.Text;
using System.Text.Json;

const string ServerHost = "127.0.0.1";
int serverPort = (args.Length > 0) ? int.Parse(args[0]) : 1234;
int clientCount = (args.Length > 1) ? int.Parse(args[1]) : 1;
string? fixedMessage = (args.Length > 2) ? args[2] : null;

Console.WriteLine($"Conectando {clientCount} cliente(s) a {ServerHost}:{serverPort}...");

Console.CancelKeyPress += (_, _) => Console.WriteLine("\n¡Hasta luego!");

async Task ConnectOnceAsync(int clientId)
{
    using TcpClient client = new TcpClient();
    await client.ConnectAsync(ServerHost, serverPort);
    Console.WriteLine($"[CLIENT - {clientId}] Conectado.");

    using NetworkStream stream = client.GetStream();
    using StreamReader reader = new StreamReader(stream, Encoding.UTF8);
    using StreamWriter writer = new StreamWriter(
        stream,
        new UTF8Encoding(false)
    ) { AutoFlush = true };
    
    string clientName;
    if (fixedMessage != null)
    {
        clientName = fixedMessage;
    }
    else
    {
        Console.Write($"[CLIENT - {clientId}] Identificate con el servidor:\n");
        clientName = Console.ReadLine() ?? $"client{clientId}";
    }

    string outgoingJSON = JsonSerializer.Serialize(
        new {type = "IDENTIFY", username = clientName}
    );
    await writer.WriteLineAsync(outgoingJSON);
    
    string? serverResponse = await reader.ReadLineAsync();

    if (serverResponse != null && serverResponse.Any(char.IsControl))
        Console.WriteLine($"[CLIENT - {clientId}] Mensaje inválido del servidor.");
    Console.WriteLine($"[CLIENT - {clientId}] <<< {serverResponse}\n");

    string? incoming;
    while((incoming = await reader.ReadLineAsync()) != null)
        Console.WriteLine($"[CLIENT - {clientId}] <<< {incoming}");
}

Task[] tasks = new Task[clientCount];
for (int i = 0; i < clientCount; i++) tasks[i] = ConnectOnceAsync(i);
await Task.WhenAll(tasks);