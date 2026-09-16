using System.Net.Sockets;
using System.Text;
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

    string outgoing;
    if (fixedMessage != null)
    {
        outgoing = fixedMessage;
    }
    else if (clientCount == 1)
    {
        Console.Write($"[CLIENT - {clientId}] Escribe un mensaje para el servidor: ");
        outgoing = Console.ReadLine() ?? "mensaje predeterminado.";
    }
    else
    {
        outgoing = $"hola desde cliente -> {clientId}.";
    }

    await writer.WriteLineAsync(outgoing);
    
    string? echoed = await reader.ReadLineAsync();

    if (echoed != null && echoed.Any(char.IsControl))
        Console.WriteLine($"[CLIENT - {clientId}] Mensaje inválido.");
    Console.WriteLine($"[CLIENT - {clientId}] Echo del servidor: {echoed}");
}

Stopwatch stopwatch = Stopwatch.StartNew();

Task[] tasks = new Task[clientCount];
for (int i = 0; i < clientCount; i++) tasks[i] = ConnectOnceAsync(i);
await Task.WhenAll(tasks);

stopwatch.Stop();
Console.WriteLine($"\nTiempo total: {stopwatch.Elapsed.TotalSeconds:F2}s para {clientCount} cliente(s).");