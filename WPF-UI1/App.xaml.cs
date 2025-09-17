﻿using System.Windows;
using Serilog;
using System.IO;
using System;

namespace WPF_UI1
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        protected override void OnStartup(StartupEventArgs e)
        {
            // 配置日志
            ConfigureLogging();
            
            Log.Information("=== 系统硬件监视器 WPF UI 启动 ===");
            Log.Information("WPF UI组件版本: 1.0.0 (与C++共享统一日志)");
            Log.Information("运行时版本: {RuntimeVersion}", Environment.Version);
            Log.Information("操作系统: {OS}", Environment.OSVersion);

            // 设置全局异常处理
            AppDomain.CurrentDomain.UnhandledException += OnUnhandledException;
            DispatcherUnhandledException += OnDispatcherUnhandledException;

            base.OnStartup(e);
        }

        protected override void OnExit(ExitEventArgs e)
        {
            Log.Information("应用程序正在退出，退出代码: {ExitCode}", e.ApplicationExitCode);
            Log.CloseAndFlush();
            base.OnExit(e);
        }

        private void ConfigureLogging()
        {
            // 使用与C++完全相同的日志文件，实现真正的统一日志
            var logDirectory = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "Logs");
            if (!Directory.Exists(logDirectory))
            {
                Directory.CreateDirectory(logDirectory);
            }

            var logFilePath = Path.Combine(logDirectory, "system_monitor.log"); // 与C++使用完全相同的日志文件

            Log.Logger = new LoggerConfiguration()
                .MinimumLevel.Debug()
                .WriteTo.Console(
                    outputTemplate: "[{Timestamp:HH:mm:ss} {Level:u3}] {Message:lj}{NewLine}{Exception}")
                .WriteTo.File(
                    logFilePath,
                    rollingInterval: RollingInterval.Day,
                    retainedFileCountLimit: 7,
                    outputTemplate: "[{Timestamp:yyyy-MM-dd HH:mm:ss.fff zzz} {Level:u3}] {Message:lj}{NewLine}{Exception}",
                    shared: true) // 允许多个进程共享同一日志文件
                .CreateLogger();

            Log.Information("WPF UI日志系统初始化完成，使用统一日志文件: {LogPath}", logFilePath);
            Log.Information("WPF与C++核心现在共享同一日志文件和控制台输出");
        }

        private void OnUnhandledException(object sender, UnhandledExceptionEventArgs e)
        {
            var exception = e.ExceptionObject as Exception;
            Log.Fatal(exception, "发生未处理的异常，程序即将终止");
            
            MessageBox.Show(
                $"发生严重错误，程序将退出：\n\n{exception?.Message}\n\n详细信息已记录到日志文件。",
                "严重错误",
                MessageBoxButton.OK,
                MessageBoxImage.Error);
        }

        private void OnDispatcherUnhandledException(object sender, System.Windows.Threading.DispatcherUnhandledExceptionEventArgs e)
        {
            Log.Error(e.Exception, "发生未处理的UI异常");
            
            MessageBox.Show(
                $"发生UI错误：\n\n{e.Exception.Message}\n\n详细信息已记录到日志文件。",
                "UI错误",
                MessageBoxButton.OK,
                MessageBoxImage.Warning);

            // 标记异常已处理，防止程序崩溃
            e.Handled = true;
        }
    }
}
