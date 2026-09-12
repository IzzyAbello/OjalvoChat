using System.Net.Sockets;
using System.Text;
using System.Diagnostics;

const string ServerHost = "127.0.0.1";
int serverPort = (args.Length > 0) ? int.Parse(args[0]) : 1234;
int clientCount = (args.Length > 1) ? int.Parse(args[1]) : 1;

Console.WriteLine($"Conectando {clientCount} cliente(s) a {ServerHost}:{serverPort}...");

async Task ConnectOnceAsync(int clientId)
{
    using TcpClient client = new TcpClient();
    await client.ConnectAsync(ServerHost, serverPort);
    Console.WriteLine($"[CLIENT - {clientId}] Conectado.");

    using NetworkStream stream = client.GetStream();
    using StreamReader reader = new StreamReader(stream, Encoding.UTF8);
    string? message = await reader.ReadLineAsync();

    if (message != null && message.Any(char.IsControl))
        Console.WriteLine($"[CLIENT - {clientId}] Mensaje inválido.");
    Console.WriteLine($"[CLIENT - {clientId}] Mensaje del servidor: {message}");
}

Stopwatch stopwatch = Stopwatch.StartNew();

Task[] tasks = new Task[clientCount];
for (int i = 0; i < clientCount; i++) tasks[i] = ConnectOnceAsync(i);
await Task.WhenAll(tasks);

stopwatch.Stop();
Console.WriteLine($"\nTiempo total: {stopwatch.Elapsed.TotalSeconds:F2}s para {clientCount} cliente(s).");