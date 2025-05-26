import subprocess
from time import sleep



EXPECTED_OUTPUT = "[info] karabo.core.Device : 'AravisTest' with deviceId: 'test' got started on server"

def _run_cmd(cmd : str) -> str:
    
    try:
        subprocess.run(cmd, check=False, shell=True, stdout=subprocess.PIPE, timeout=10, stderr=subprocess.STDOUT)
    except subprocess.TimeoutExpired as err:
        # cleanup
        subprocess.run("killall -9 karabo-aravis-server", shell=True)
        return err.stdout.decode()[:-1]
    return ""


def test_cppserver():
    output =_run_cmd("karabo-aravis-server init='{\"AravisBaslerCamera\": {\"deviceId\": \"test\", \"classId\": \"AravisBaslerCamera\", \"cameraId\": \"192.168.0.1\"}}'")
    assert EXPECTED_OUTPUT in output
