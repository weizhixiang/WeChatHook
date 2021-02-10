using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using WeChat;
using WeChat.Helper;
namespace WeChatRobot
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private int pid;

        private WeChatSdk WeChat;
        public delegate void ShowLogDelegate(string msg);
        public ShowLogDelegate ShowLogEvent;
        public ObservableCollection<ContactModel> Contacts { get; set; }
        public ObservableCollection<MessageModel> ReceiveMsgs { get; set; }

        public MainWindow()
        {
            InitializeComponent();
            WeChat = new WeChatSdk();
            Contacts = new ObservableCollection<ContactModel>();
            ReceiveMsgs = new ObservableCollection<MessageModel>();
            DataContext = this;
            ShowLogEvent = (msg) =>
            {
                this.tb_Log.AppendText(msg);
                this.tb_Log.AppendText(Environment.NewLine);
                this.tb_Log.ScrollToEnd();
            };
            WeChat.LogEvent += WeChat_LogEvent;
            WeChat.ReceiveContactEvent += WeChat_ReceiveContactEvent;
            WeChat.ReceiveOtherIMEvent += WeChat_ReceiveOtherIMEvent;
            WeChat.WeChatInitEvent += WeChat_WeChatInitEvent;
            WeChat.ConnetionCloseEvent += WeChat_ConnetionCloseEvent;
            btn_OpenWeChat.Click += Btn_OpenWeChat_Click;
            btn_UserInfo.Click += WeChat_Refresh;
            btn_SendText.Click += WeChat_SendText;
        }


        private void WeChat_ConnetionCloseEvent(object sender, WeChat.Events.ConnetionCloseArgs e)
        {
            ShowLog($"[日志]websocket连接断开,请检查微信是否正常运行。");
        }

        private void WeChat_WeChatInitEvent(object sender, WeChat.Events.WeChatInitArgs e)
        {
            pid = e.PorcessId;
            ShowLog($"[日志]{e.Message}");

            
            while (!WeChat.CheckLoginState(e.PorcessId))
            {
                Thread.Sleep(1000);
            }
            //var user = WeChat.GetLoginUser(e.PorcessId);
            //if (user != null)
            //{
            //    this.Dispatcher.BeginInvoke((Action)delegate ()
            //    {
            //        tb_UserInfo.Text = QNSerialize.SerializeObject(user);
            //        lb_Status.Content = "已登录";
            //    });
            //}
            this.Dispatcher.BeginInvoke((Action)delegate ()
            {
                Contacts.Clear();
                lb_Status.Content = "已登录";
            });
            WeChat.GetContactList(pid);
        }

        private void WeChat_ReceiveOtherIMEvent(object sender, WeChat.Events.ReceiveOtherIMArgs e)
        {
            if (e != null)
            {
                var model = new MessageModel()
                {
                    Type = e.Type,
                    Source=e.Source,
                    Wxid=e.Wxid,
                    MsgSender=e.MsgSender,
                    Content=e.Content,
                    IsMoney=e.IsMoney
                };
                this.Dispatcher.BeginInvoke((Action)delegate ()
                {
                    ReceiveMsgs.Add(model);
                });
                ShowLog($"[消息]{e.Content}");
            }

        }

        private void WeChat_ReceiveContactEvent(object sender, Contact e)
        {
            if (e != null)
            {
                var model = new ContactModel()
                {
                    Alias = e.Alias,
                    BigHeadImgUrl = e.BigHeadImgUrl,
                    NickName = e.NickName,
                    Remark = e.Remark,
                    UserName = e.UserName
                };
                this.Dispatcher.BeginInvoke((Action)delegate ()
                {
                    Contacts.Add(model);
                });
            }
        }

        private void WeChat_LogEvent(object sender, string e)
        {
            ShowLog($"[日志]{e}");
        }
        private void WeChat_Refresh(object sender, RoutedEventArgs e)
        {
            if (pid != 0)
            {
                if (WeChat.CheckLoginState(pid))
                {
                    this.Dispatcher.BeginInvoke((Action)delegate ()
                    {
                        Contacts.Clear();
                    });
                    WeChat.GetContactList(pid);
                }
                else
                {
                    ShowLog($"[日志]无法刷新，未登录");
                }
               
            }
            else
            {
                ShowLog($"[日志]无法刷新，未打开微信或注入");
            }

        }
        private void WeChat_SendText(object sender, RoutedEventArgs e)
        {

            WeChat.SendTextMessage(pid, tb_Param1.Text, tb_Content.Text);
            ShowLog($"[发送消息]{tb_Param1.Text},{tb_Content.Text}");
        }
        private void Btn_OpenWeChat_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                var ret = WeChat.OpenWechat();
                var list = Process.GetProcessesByName("WeChat");
                if (list.Length > 0)
                    ret = list[0].Id;
                if (ret == 0) return;
                if (!WeChat.IsInjected(ret))
                {
                    WeChat.InjectDll(pid);
                }
            }
            catch (Exception ex)
            {
                ShowLog($"[异常]{ex.Message}");
            }

        }

        public void ShowLog(string msg)
        {
            if (string.IsNullOrEmpty(msg))
            {
                return;
            }
            if (!msg.StartsWith(DateTime.Now.Year.ToString(CultureInfo.InvariantCulture)))
            {
                msg = string.Format("{0} {1}", DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss"), msg);
            }
            msg = msg.Replace("\r\n", Environment.NewLine)
                .Replace("\n", Environment.NewLine)
                .Replace("\r", Environment.NewLine);
            this.Dispatcher.BeginInvoke(ShowLogEvent, msg);

        }

    }

    public class MessageModel : INotifyPropertyChanged
    {
        public string Type { get; set; }
        public string Source { get; set; }
        public string Wxid { get; set; }
        public string MsgSender { get; set; }
        public string Content { get; set; }
        public bool IsMoney { get; set; }

        public event PropertyChangedEventHandler PropertyChanged;
    }

    public class ContactModel : INotifyPropertyChanged
    {
        private string userName;

        public string UserName
        {
            get { return userName; }
            set
            {
                if (userName != value)
                {
                    userName = value;
                    OnPropertyChanged("UserName");
                }
            }
        }

        public string Alias { get; set; }

        public string NickName { get; set; }

        public string Remark { get; set; }

        public string BigHeadImgUrl { get; set; }



        public event PropertyChangedEventHandler PropertyChanged;
        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
