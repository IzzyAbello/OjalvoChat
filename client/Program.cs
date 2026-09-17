using System.Net.Sockets;
using System.Text;
using System.Text.Json;
using System.Diagnostics;

const string ServerHost = "127.0.0.1";
int serverPort = (args.Length > 0) ? int.Parse(args[0]) : 1234;
int clientCount = (args.Length > 1) ? int.Parse(args[1]) : 1;
string? fixedMessage = (args.Length > 2) ? args[2] : null;

Console.WriteLine($"Conectando {clientCount} cliente(s) a {ServerHost}:{serverPort}...");

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
    
    string? greeting = await reader.ReadLineAsync();
    Console.WriteLine($"[CLIENT - {clientId}] Saludo del servidor: {greeting}");

    string clientName;
    if (fixedMessage != null)
    {
        clientName = fixedMessage;
    }
    else if (clientCount == 1)
    {
        Console.Write($"[CLIENT - {clientId}] Identificate con el servidor: ");
        clientName = Console.ReadLine() ?? $"client{clientId}";
    }
    else
    {
        clientName = $"client{clientId}.";
    }

    string outgoingJSON = JsonSerializer.Serialize(
        new {type = "IDENTIFY", username = clientName}
    );
    await writer.WriteLineAsync(outgoingJSON);
    
    string? serverResponse = await reader.ReadLineAsync();

    if (serverResponse != null && serverResponse.Any(char.IsControl))
        Console.WriteLine($"[CLIENT - {clientId}] Mensaje inválido del servidor.");
    Console.WriteLine($"[CLIENT - {clientId}] Respuesta del servidor: {serverResponse}");
}

Stopwatch stopwatch = Stopwatch.StartNew();

Task[] tasks = new Task[clientCount];
for (int i = 0; i < clientCount; i++) tasks[i] = ConnectOnceAsync(i);
await Task.WhenAll(tasks);

stopwatch.Stop();
Console.WriteLine($"\nTiempo total: {stopwatch.Elapsed.TotalSeconds:F2}s para {clientCount} cliente(s).");