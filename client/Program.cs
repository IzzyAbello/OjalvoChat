using System.Net.Sockets;
using System.Text;

const string ServerHost = "127.0.0.1";
const int ServerPort = 1234;

using TcpClient client = new TcpClient();
Console.WriteLine($"Conectando a {ServerHost}:{ServerPort}...");
client.Connect(ServerHost, ServerPort);
Console.WriteLine("Conectado.");

using NetworkStream stream = client.GetStream();

byte[] buffer = new byte[255];
int bytesRead = stream.Read(buffer, 0, buffer.Length);

string message = Encoding.UTF8.GetString(buffer, 0, bytesRead);
int nullIndex = message.IndexOf('\0');
if (nullIndex >= 0)
{
    message = message[..nullIndex];
}

Console.WriteLine($"Mensaje del servidor: {message}");