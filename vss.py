import os
import subprocess
import ctypes

# 检查是否为管理员权限
def is_admin():
    try:
        return ctypes.windll.shell32.IsUserAnAdmin()
    except:
        return False

# 运行命令并显示输出
def run_cmd(cmd):
    print(f"\n>>> 执行: {cmd}")
    result = subprocess.run(cmd, shell=True, text=True, capture_output=True)
    if result.stdout:
        print(result.stdout.strip())
    if result.stderr:
        print(result.stderr.strip())

# 主修复流程
def main():
    print("=== VSS 快速修复开始 ===")

    # 停止相关服务
    run_cmd("net stop vss")
    run_cmd("net stop swprv")

    # 注册必要的 DLL
    dlls = [
        "ole32.dll", "oleaut32.dll", "vss_ps.dll",
        "vswriter.dll", "swprv.dll"
    ]
    for dll in dlls:
        run_cmd(f"regsvr32 /s {dll}")

    # 启动服务
    run_cmd("net start swprv")
    run_cmd("net start vss")

    # 删除所有现有快照
    run_cmd("vssadmin delete shadows /all /quiet")

    # 显示 VSS 写入器状态
    run_cmd("vssadmin list writers")

    print("\n=== 修复完成，请重启计算机后重试备份 ===")

if __name__ == "__main__":
    if is_admin():
        main()
    else:
        print("⚠️ 当前不是管理员权限，正在尝试以管理员身份重新运行...")
        ctypes.windll.shell32.ShellExecuteW(None, "runas", "python", __file__, None, 1)
