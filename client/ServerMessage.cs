using System.Text.Json;

namespace ChatClient;

public abstract class ServerMessage
{
    public abstract void Display(View view);

    public static ServerMessage? Parse(string line)
    {
        try
        {
            using JsonDocument doc = JsonDocument.Parse(line);
            JsonElement root = doc.RootElement;

            if (!root.TryGetProperty("type", out JsonElement typeProp))
                return null;

            string type = typeProp.GetString() ?? "";

            string? Get(string name) =>
                root.TryGetProperty(name, out JsonElement p) ? p.GetString() : null;

            Dictionary<string, string> GetUsers()
            {
                var users = new Dictionary<string, string>();
                if (root.TryGetProperty("users", out JsonElement obj))
                    foreach (JsonProperty prop in obj.EnumerateObject())
                        users[prop.Name] = prop.Value.GetString() ?? "";
                return users;
            }

            return type switch
            {
                "RESPONSE"         => new ResponseMessage
                (
                    Get("operation") ?? "",
                    Get("result") ?? "",
                    Get("extra")
                ),
                "NEW_USER"         => new NewUserMessage(Get("username") ?? ""),
                "NEW_STATUS"       => new NewStatusMessage
                (
                    Get("username") ?? "",
                    Get("status") ?? ""
                ),
                "USER_LIST"        => new UserListMessage(GetUsers()),
                "TEXT_FROM"        => new TextFromMessage
                (
                    Get("username") ?? "", 
                    Get("text") ?? ""
                ),
                "PUBLIC_TEXT_FROM" => new PublicTextFromMessage
                (
                    Get("username") ?? "",
                    Get("text") ?? ""
                ),
                "INVITATION"       => new InvitationMessage
                (
                    Get("username") ?? "",
                    Get("roomname") ?? ""
                ),
                "JOINED_ROOM"      => new JoinedRoomMessage
                (
                    Get("roomname") ?? "",
                    Get("username") ?? ""
                ),
                "ROOM_USER_LIST"   => new RoomUserListMessage
                (
                    Get("roomname") ?? "", 
                    GetUsers()
                ),
                "ROOM_TEXT_FROM"   => new RoomTextFromMessage
                (
                    Get("roomname") ?? "",
                    Get("username") ?? "",
                    Get("text") ?? ""
                ),
                "LEFT_ROOM"        => new LeftRoomMessage
                (
                    Get("roomname") ?? "",
                    Get("username") ?? ""
                ),
                "DISCONNECTED"     => new DisconnectedMessage(Get("username") ?? ""),
                _                  => null,
            };
        }
        catch (JsonException)
        {
            return null;
        }
    }
}

public class ResponseMessage : ServerMessage
{
    public string Operation { get; }
    public string Result { get; }
    public string? Extra { get; }

    public ResponseMessage(string operation, string result, string? extra)
    {
        Operation = operation;
        Result = result;
        Extra = extra;
    }

    public override void Display(View view) => view.ShowResponse(Operation, Result, Extra);
}

public class NewUserMessage : ServerMessage
{
    public string Username { get; }
    public NewUserMessage(string username) => Username = username;
    public override void Display(View view) => view.ShowNewUser(Username);
}

public class NewStatusMessage : ServerMessage
{
    public string Username { get; }
    public string Status { get; }
    public NewStatusMessage(string username, string status)
    {
        Username = username;
        Status = status;
    }
    public override void Display(View view) => view.ShowNewStatus(Username, Status);
}

public class UserListMessage : ServerMessage
{
    public Dictionary<string, string> Users { get; }
    public UserListMessage(Dictionary<string, string> users) => Users = users;
    public override void Display(View view) => view.ShowUserList(Users);
}

public class TextFromMessage : ServerMessage
{
    public string Username { get; }
    public string Text { get; }
    public TextFromMessage(string username, string text)
    {
        Username = username;
        Text = text;
    }
    public override void Display(View view) => view.ShowPrivateText(Username, Text);
}

public class PublicTextFromMessage : ServerMessage
{
    public string Username { get; }
    public string Text { get; }
    public PublicTextFromMessage(string username, string text)
    {
        Username = username;
        Text = text;
    }
    public override void Display(View view) => view.ShowPublicText(Username, Text);
}

public class InvitationMessage : ServerMessage
{
    public string Username { get; }
    public string RoomName { get; }
    public InvitationMessage(string username, string roomName) 
    {
        Username = username;
        RoomName = roomName;
    }
    public override void Display(View view) => view.ShowInvitation(Username, RoomName);
}

public class JoinedRoomMessage : ServerMessage
{
    public string RoomName { get; }
    public string Username { get; }
    public JoinedRoomMessage(string roomName, string username)
    {
        RoomName = roomName;
        Username = username;
    }
    public override void Display(View view) => view.ShowJoinedRoom(RoomName, Username);
}

public class RoomUserListMessage : ServerMessage
{
    public string RoomName { get; }
    public Dictionary<string, string> Users { get; }
    public RoomUserListMessage(string roomName, Dictionary<string, string> users)
    {
        RoomName = roomName;
        Users = users;
    }
    public override void Display(View view) => view.ShowRoomUserList(RoomName, Users);
}

public class RoomTextFromMessage : ServerMessage
{
    public string RoomName { get; }
    public string Username { get; }
    public string Text { get; }
    public RoomTextFromMessage(string roomName, string username, string text)
    {
        RoomName = roomName;
        Username = username;
        Text = text;
    }
    public override void Display(View view) => view.ShowRoomText(RoomName, Username, Text);
}

public class LeftRoomMessage : ServerMessage
{
    public string RoomName { get; }
    public string Username { get; }
    public LeftRoomMessage(string roomName, string username) 
    {
        RoomName = roomName;
        Username = username; 
    }
    public override void Display(View view) => view.ShowLeftRoom(RoomName, Username);
}

public class DisconnectedMessage : ServerMessage
{
    public string Username { get; }
    public DisconnectedMessage(string username) => Username = username;
    public override void Display(View view) => view.ShowDisconnected(Username);
}