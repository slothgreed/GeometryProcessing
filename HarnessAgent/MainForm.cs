namespace HarnessAgent;

public sealed class MainForm : Form
{
    private readonly TextBox input = new()
    {
        Multiline = true,
        Dock = DockStyle.Fill,
        ScrollBars = ScrollBars.Vertical,
        PlaceholderText = "ここに文字を入力してください。"
    };
    private readonly TextBox answer = new()
    {
        Multiline = true,
        ReadOnly = true,
        Dock = DockStyle.Fill,
        ScrollBars = ScrollBars.Vertical,
        BackColor = SystemColors.Window
    };
    private readonly Label status = new() { Text = "入力して送信してください。", AutoSize = true };

    public MainForm()
    {
        Text = "HarnessAgent — 応答デモ";
        StartPosition = FormStartPosition.CenterScreen;
        ClientSize = new Size(660, 460);
        MinimumSize = new Size(480, 380);
        AutoScaleMode = AutoScaleMode.Dpi;

        var layout = new TableLayoutPanel
        {
            Dock = DockStyle.Fill,
            Padding = new Padding(20),
            ColumnCount = 1,
            RowCount = 7
        };
        layout.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100));
        layout.RowStyles.Add(new RowStyle(SizeType.Absolute, 44));
        layout.RowStyles.Add(new RowStyle(SizeType.Absolute, 28));
        layout.RowStyles.Add(new RowStyle(SizeType.Percent, 40));
        layout.RowStyles.Add(new RowStyle(SizeType.Absolute, 48));
        layout.RowStyles.Add(new RowStyle(SizeType.Absolute, 28));
        layout.RowStyles.Add(new RowStyle(SizeType.Percent, 60));
        layout.RowStyles.Add(new RowStyle(SizeType.Absolute, 28));

        var send = new Button { Text = "送信", AutoSize = true, Anchor = AnchorStyles.Right, Height = 34 };
        send.Click += Send;
        layout.Controls.Add(new Label
        {
            Text = "固定応答デモ：入力内容にかかわらず同じ回答を返します。API通信・課金はありません。",
            Dock = DockStyle.Fill
        }, 0, 0);
        layout.Controls.Add(new Label { Text = "入力", AutoSize = true }, 0, 1);
        layout.Controls.Add(input, 0, 2);
        layout.Controls.Add(send, 0, 3);
        layout.Controls.Add(new Label { Text = "回答", AutoSize = true }, 0, 4);
        layout.Controls.Add(answer, 0, 5);
        layout.Controls.Add(status, 0, 6);
        Controls.Add(layout);
    }

    private void Send(object? sender, EventArgs e)
    {
        // First UI exercise: replace all user input with a fixed request.
        // No Claude API, model, or API key is involved here.
        var request = "こんにちは";
        answer.Text = GetDemoReply(request);
        status.Text = $"回答しました（{DateTime.Now:HH:mm:ss}）。";
    }

    private static string GetDemoReply(string request)
    {
        return $"「{request}」を受け取りました。\r\nHarnessAgentのデモ応答です。";
    }
}
