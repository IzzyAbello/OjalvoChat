using ChatClient;

int serverPort = (args.Length > 0) ? int.Parse(args[0]) : 1234;
string serverHost = (args.Length > 1) ? args[1] : "127.0.0.1";

View view = new View();
view.ShowWelcome();

Controller controller = new Controller(view, serverHost, serverPort);
await controller.RunAsync();