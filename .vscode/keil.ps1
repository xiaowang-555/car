# Keil uVision 命令行封装，供 VSCode Task 调用
# 用法: .\.vscode\keil.ps1 -Mode build|rebuild|flash|download
param(
    [ValidateSet('build', 'rebuild', 'flash', 'download')]
    [string]$Mode = 'build'
)

$ErrorActionPreference = 'Stop'

$Root = Split-Path $PSScriptRoot -Parent          # 工程根目录（.vscode 的上一级）
$UV4  = 'D:\keil5\stm32data\UV4\UV4.exe'
$Proj = Join-Path $Root 'Project.uvprojx'
$Log  = Join-Path $Root 'Objects\keil.log'        # Objects/ 已被 gitignore，日志放这里不污染仓库

if (-not (Test-Path $UV4)) {
    Write-Host "找不到 UV4.exe: $UV4" -ForegroundColor Red
    exit 20
}
if (-not (Test-Path $Proj)) {
    Write-Host "找不到工程文件: $Proj" -ForegroundColor Red
    exit 21
}

function Invoke-UV4 {
    param([string]$Arg)
    # UV4 的 -o 不会覆盖已有日志，先删掉，避免读到上一次的残留结果
    if (Test-Path $Log) { Remove-Item $Log -Force }
    $p = Start-Process -FilePath $UV4 `
                       -ArgumentList $Arg, "`"$Proj`"", '-o', "`"$Log`"" `
                       -Wait -PassThru -NoNewWindow
    return $p.ExitCode
}

function Show-Log {
    if (-not (Test-Path $Log)) { return }
    # Keil 日志是 GBK 编码，按 936 代码页读，否则中文注释会显示成乱码
    $enc  = [System.Text.Encoding]::GetEncoding(936)
    $text = [System.IO.File]::ReadAllText($Log, $enc)
    Write-Host $text.TrimEnd()
}

# UV4 退出码: 0=无错无警告  1=有警告  2=有错误  3=致命错误  其余=工具/授权问题
function Get-ExitCode {
    param([int]$Code)
    switch ($Code) {
        0 { Write-Host '成功：无错误，无警告。' -ForegroundColor Green;  return 0 }
        1 { Write-Host '成功：有警告（见上）。' -ForegroundColor Yellow; return 0 }
        default {
            Write-Host "失败：UV4 退出码 $Code。" -ForegroundColor Red
            return $Code
        }
    }
}

# 首次烧录常因 stlinkserver 正在初始化而报 "Target DLL has been cancelled"，
# 属瞬时故障，重试一次即可 —— 不是工程配置问题（2026-09-26 实测）。
function Invoke-FlashWithRetry {
    $c = Invoke-UV4 '-f'
    Show-Log
    if ($c -gt 1) {
        Write-Host ''
        Write-Host '首次烧录失败，2 秒后重试一次（stlinkserver 瞬态占用）...' -ForegroundColor Yellow
        Start-Sleep -Seconds 2
        $c = Invoke-UV4 '-f'
        Show-Log
    }
    return $c
}

switch ($Mode) {

    'build' {
        Write-Host '=== 增量编译 ===' -ForegroundColor Cyan
        $c = Invoke-UV4 '-b'
        Show-Log
        exit (Get-ExitCode $c)
    }

    'rebuild' {
        Write-Host '=== 完整重建 ===' -ForegroundColor Cyan
        exit (Get-ExitCode (Invoke-UV4 '-r'))
    }

    'flash' {
        Write-Host '=== 编译 ===' -ForegroundColor Cyan
        $c = Invoke-UV4 '-b'
        Show-Log
        if ($c -gt 1) {
            Write-Host '编译未通过，已中止烧录。' -ForegroundColor Red
            exit (Get-ExitCode $c)
        }
        Write-Host ''
        Write-Host '=== 烧录 ===' -ForegroundColor Cyan
        exit (Get-ExitCode (Invoke-FlashWithRetry))
    }

    'download' {
        Write-Host '=== 仅烧录（不编译）===' -ForegroundColor Cyan
        exit (Get-ExitCode (Invoke-FlashWithRetry))
    }
}
