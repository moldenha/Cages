import requests

def exists(url):
    response = requests.get(url)
    if response.status_code == 200:
        return True
    return False

def check(base, start, end):
    for i in range(start, (end+1)):
        n_url = base + str(i)
        print("checking {}".format(n_url))
        if(exists(n_url)):
            return n_url
    return None

if __name__ == '__main__':
    base = "http://192.168.1."
    n_url = check(base, 100, 170)
    if(n_url != None):
        print("Found URL: {}".format(n_url))
    else:
        print("URL not found")
