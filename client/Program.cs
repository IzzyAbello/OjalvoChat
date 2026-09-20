using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Text.Json;

const string ServerHost = "127.0.0.1";
int serverPort = (args.Length > 0) ? int.Parse(args[0]) : 1234;
int clientCount = (args.Length > 1) ? int.Parse(args[1]) : 1;

Console.WriteLine($"Conectando {clientCount} cliente(s) a {ServerHost}:{serverPort}...");

Console.CancelKeyPress += (_, _) => Console.WriteLine("\n¡Hasta luego!");

SemaphoreSlim consoleLock = new SemaphoreSlim(1, 1);

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


    bool disconnect = false;
    while (!disconnect)
    {
        await consoleLock.WaitAsync();
        try
        {
            Console.WriteLine($"[CLIENT - {clientId}] Escribe el tipo de mensaje:");
            string messageType = Console.ReadLine()?.Trim().ToLower() ?? "";

            string outgoingJson;
            switch (messageType)
            {
                case "disconnect":
                    Console.WriteLine($"[CLIENT - {clientId}] Desconectando...");
                    outgoingJson = JsonSerializer.Serialize(
                        new {type = "DISCONNECT"}
                    );
                    disconnect = true;
                    break;
                case "identify":
                    Console.WriteLine($"[CLIENT - {clientId}] Escribe tu username:");
                    string username = Console.ReadLine() ?? "";
                    outgoingJson = JsonSerializer.Serialize(
                        new { type = "IDENTIFY", username }
                    );
                    break;
                case "status":
                    Console.WriteLine($"[CLIENT - {clientId}] Escribe tu status:");
                    string status = Console.ReadLine() ?? "";
                    outgoingJson = JsonSerializer.Serialize(
                        new { type = "STATUS", status }
                    );
                    break;
                default:
                    Console.WriteLine( $"[CLIENT - {clientId}] Tipo de mensaje inválido." );
                    continue;
            }

            Console.WriteLine($"[CLIENT - {clientId}] >>> {outgoingJson}");
            await writer.WriteLineAsync(outgoingJson);


            char[] buffer = new char[1024*1024];
            int bytesRead = await reader.ReadAsync(buffer, 0, buffer.Length)
                .WaitAsync(TimeSpan.FromMilliseconds(3000));
            string serverResponse = new string(buffer, 0, bytesRead);

            if (bytesRead <= 0)
                Console.WriteLine($"[CLIENT - {clientId}] No llegan respuestas del server...");
            else Console.WriteLine($"[CLIENT - {clientId}] <<< {serverResponse}");
        }
        finally
        {
            consoleLock.Release();
        }
    }
}

Task[] tasks = new Task[clientCount];
for (int i = 0; i < clientCount; i++) tasks[i] = ConnectOnceAsync(i);
await Task.WhenAll(tasks);