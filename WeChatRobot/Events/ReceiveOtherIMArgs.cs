using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace WeChat.Events
{
    public class ReceiveOtherIMArgs
    {
        public string Type { get; set; }

        public string Source { get; set; }

        public string Wxid { get; set; }

        public string MsgSender { get; set; }
        public string Content { get; set; }

        public bool IsMoney { get; set; }

    }
}
