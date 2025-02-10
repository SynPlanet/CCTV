using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Windows;

namespace Cam
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        protected override void OnStartup(StartupEventArgs e)
        {
            AppDomain.CurrentDomain.UnhandledException += new UnhandledExceptionEventHandler(CurrentDomain_UnhandledException);
            if (e.Args.Length == 0) Folder = null;
            else Folder = e.Args[0];
            if (e.Args.Length == 2 && e.Args[1].ToLower() == "debug") Cam.MainWindow.Writer = new LogToFile("LogMsg.txt");
            else Cam.MainWindow.Writer = new LogEmpty();

            base.OnStartup(e);
            App.Current.ShutdownMode = ShutdownMode.OnMainWindowClose;
        }

        void CurrentDomain_UnhandledException(object sender, UnhandledExceptionEventArgs e)
        {
            Exception ex = e.ExceptionObject as Exception;
            while (ex != null)
            {
                WriteUnhandledException(Cam.MainWindow.Writer, ex);
                ex = ex.InnerException;
            }
        }

        private void WriteUnhandledException(Log writer, Exception ex)
        {
            if (writer == null || ex == null) return;
            writer.WriteLine(ex.Message);
            writer.WriteLine();
            writer.WriteLine(ex.StackTrace);
            writer.WriteLine("----------------");
        }

        public string Folder { get; set; }
    }
}
