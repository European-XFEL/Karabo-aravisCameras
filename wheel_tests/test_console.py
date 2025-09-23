import subprocess
import re


EXPECTED_OUTPUT= r"^.*\[info\] karabo\.core\.Device : 'AravisBaslerCamera' \(version '.*'\) with deviceId: 'MyTestCamera' got started on server.*$"

def _run_cmd(cmd: str) -> str:
    """
    Runs cmd in the shell, returns its combined stdout/stderr.
    If it times out, kills the server and still returns whatever was in stdout.
    """
    try:
        proc = subprocess.run(
            cmd,
            shell=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            timeout=10,
            check=False,
        )
        # strip a trailing newline
        return proc.stdout.decode().rstrip("\n")
    except subprocess.TimeoutExpired as err:
        # cleanup
        subprocess.run("killall -9 karabo-aravis-cameras-server", shell=True)
        return err.stdout.decode().rstrip("\n")

def test_cppserver():
    output =_run_cmd("karabo-aravis-cameras-server init='{\"MyTestCamera\": {\"classId\": \"AravisBaslerCamera\", \"cameraId\": \"192.168.0.1\"}}'")
    assert re.findall(EXPECTED_OUTPUT, output, re.MULTILINE), output
