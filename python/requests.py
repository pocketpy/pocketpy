class Response:
    def __init__(self, status_code, reason, content):
        self.status_code = status_code
        self.reason = reason
        self.content = content

        assert type(self.status_code) is int
        assert type(self.reason) is str
        assert type(self.content) is bytes

    @property
    def text(self):
        return self.content.decode()
    
    def __repr__(self):
        code = self.status_code
        return f'<Response [{code}]>'

def _parse_h(headers):
    if headers is None:
        return []
    if type(headers) is dict:
        return list(headers.items())
    raise ValueError('headers must be dict or None')

def get(url, headers=None):
    headers = _parse_h(headers)
    return _request('GET', url, headers, None)

def post(url, data: bytes, headers=None):
    headers = _parse_h(headers)
    return _request('POST', url, headers, data)

def put(url, data: bytes, headers=None):
    headers = _parse_h(headers)
    return _request('PUT', url, headers, data)

def delete(url, headers=None):
    headers = _parse_h(headers)
    return _request('DELETE', url, headers, None)