namespace ChatClient;

public class View
{
    public void ShowHelp()
    {
        WriteLine
        (
            ConsoleColor.Cyan,
            "💡 COMANDOS:\n"
            + "🛠️ help         ➡️  Ayuda del programa.\n"
            + "🔌 disconnect   ➡️  Para desconectarse del servidor.\n"
            + "🪪 identify     ➡️  Para identificarse con el servidor.\n"
            + "👤 status       ➡️  Para cambiar el estado (ACTIVE, AWAY, BUSY).\n"
            + "👥 users        ➡️  Para solicitar la lista de usuarios en el servidor.\n"
            + "🔓 text         ➡️  Para enviar un mensaje privado a un usuario.\n"
            + "📢 public       ➡️  Para escribir un mensaje en el chat general.\n"
            + "🔑 new room     ➡️  Para crear una nueva sala privada.\n"
            + "🚪 join         ➡️  Para unirse a una sala privada.\n"
            + "👤 room users   ➡️  Para solicitar la lista de usuarios en la sala privada.\n"
            + "🗣️ room text    ➡️  Para escribir un mensaje en la sala privada.\n"
            + "🏃‍♂️ leave room   ➡️  Para abandonar alguna sala privada.\n"
            + "🎟️ invite       ➡️  Para invitar usuarios a la sala privada.\n"
        );
    }
    public void ShowWelcome()
    {
        Console.ForegroundColor = ConsoleColor.Magenta;
        Console.WriteLine("💬 ¡Bienvenido al cliente de OjalvoChat! 💬");
        Console.ResetColor();
    }

    public void ShowConnecting(string host, int port) =>
        WriteLine(ConsoleColor.DarkYellow, $"🔌 Conectando a {host}:{port}...");

    public void ShowConnected() =>
        WriteLine(ConsoleColor.Green, "✅ Conectado.");

    public void ShowConnectionError(Exception ex) =>
        WriteLine(ConsoleColor.Red, $"❌ No se pudo conectar: {ex.Message}");

    public void ShowGoodbye() =>
        WriteLine(ConsoleColor.Magenta, "👋 ¡Hasta luego!");

    public void ShowServerClosed() =>
        WriteLine(ConsoleColor.Red, "🔌 El servidor cerró la conexión.");

    public void ShowInvalidMessage(string raw) =>
        WriteLine(ConsoleColor.DarkRed, $"❔ Llegó algo raro del servidor: {raw}");

    public void ShowInvalidCommand() =>
        WriteLine(ConsoleColor.Yellow, "⚠️  Tipo de mensaje inválido.");

    public void ShowOutgoing(string type) =>
        WriteLine(ConsoleColor.DarkGray, $">>> Enviando mensaje de tipo {type}...");


    public string? AskMessageType() => Prompt("Escribe el tipo de mensaje");
    public string? AskUsername() => Prompt("Escribe tu username");
    public string? AskStatus() => Prompt("Escribe tu status");
    public string? AskRecipient() => Prompt("Escribe el destinatario");
    public string? AskText() => Prompt("Escribe el texto");
    public string? AskPublicText() => Prompt("Escribe el texto público");
    public string? AskRoomName() => Prompt("Escribe el nombre del cuarto");
    public string? AskGuests() => Prompt("Escribe los nombres de los invitados separados por comas");

    private string? Prompt(string label)
    {
        Console.ForegroundColor = ConsoleColor.Cyan;
        Console.WriteLine($"❯ {label}:");
        Console.ResetColor();
        return Console.ReadLine();
    }

    public void ShowResponse(string operation, string result, string? extra)
    {
        result = (result == "SUCCESS") ? "ÉXITO" : "FRACASO"; 
        string e = extra ?? "";
        ConsoleColor color = (result == "ÉXITO") ? ConsoleColor.Green : ConsoleColor.Red;
        WriteLine
        (
            color,
            $"📨 La Operación {operation} resultó en {result} " + ((e == "") ? "" : $"con {e}.")
        );
    }

    public void ShowNewUser(string username) =>
        WriteLine(ConsoleColor.Green, $"🆕 {username} se ha conectado.");

    public void ShowNewStatus(string username, string status) =>
        WriteLine(ConsoleColor.Yellow, $"{StatusEmoji(status)} {username} cambió su estado a {status}.");

    public void ShowUserList(Dictionary<string, string> users)
    {
        WriteLine(ConsoleColor.Cyan, "👥 Usuarios conectados:");
        foreach ((string name, string status) in users)
            WriteLine(ConsoleColor.Cyan, $"   {StatusEmoji(status)} {name} ({status})");
    }

    public void ShowPrivateText(string username, string text) =>
        WriteLine(ConsoleColor.Magenta, $"💌 {username} (privado): {text}");

    public void ShowPublicText(string username, string text) =>
        WriteLine(ConsoleColor.White, $"📢 {username}: {text}");

    public void ShowInvitation(string username, string roomName) =>
        WriteLine(ConsoleColor.Yellow, $"📩 {username} te invitó a la sala «{roomName}».");

    public void ShowJoinedRoom(string roomName, string username) =>
        WriteLine(ConsoleColor.Green, $"🚪 {username} se unió a la sala «{roomName}».");

    public void ShowRoomUserList(string roomName, Dictionary<string, string> users)
    {
        WriteLine(ConsoleColor.Cyan, $"👥🏠 Usuarios en «{roomName}»:");
        foreach ((string name, string status) in users)
            WriteLine(ConsoleColor.Cyan, $"   {StatusEmoji(status)} {name} ({status})");
    }

    public void ShowRoomText(string roomName, string username, string text) =>
        WriteLine(ConsoleColor.Blue, $"💬 [{roomName}] {username}: {text}");

    public void ShowLeftRoom(string roomName, string username) =>
        WriteLine(ConsoleColor.DarkYellow, $"🏃 {username} salió de la sala «{roomName}».");

    public void ShowDisconnected(string username) =>
        WriteLine(ConsoleColor.DarkYellow, $"🔌 {username} se desconectó.");

    private static string StatusEmoji(string status) => status switch
    {
        "ACTIVE" => "🟢",
        "AWAY"   => "🟡",
        "BUSY"   => "🔴",
        _        => "⚪",
    };

    private static void WriteLine(ConsoleColor color, string text)
    {
        Console.ForegroundColor = color;
        Console.WriteLine(text);
        Console.ResetColor();
    }
}