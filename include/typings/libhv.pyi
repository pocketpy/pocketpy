from typing import Literal, Generator, Callable, Union

WsChannelId = int
HttpStatusCode = int
HttpHeaders = dict[str, str]
ErrorCode = int

class Future[T]:
    @property
    def completed(self) -> bool: ...
    def cancel(self) -> None: ...
    def __iter__(self) -> Generator[T, None, None]: ...

class HttpResponse(Future['HttpResponse']):
    @property
    def status_code(self) -> int: ...
    @property
    def headers(self) -> dict[str, str]: ...
    @property
    def text(self) -> str: ...
    @property
    def content(self) -> bytes: ...

    def json(self): ...


class HttpClient:
    def get(self, url: str, /, params=None, headers=None, timeout=10) -> HttpResponse: ...
    def post(self, url: str, /, params=None, headers=None, data=None, json=None, timeout=10) -> HttpResponse: ...
    def put(self, url: str, /, params=None, headers=None, data=None, json=None, timeout=10) -> HttpResponse: ...
    def delete(self, url: str, /, params=None, headers=None, timeout=10) -> HttpResponse: ...


class HttpRequest:
    @property
    def method(self) -> Literal['GET', 'POST', 'PUT', 'DELETE']: ...
    @property
    def path(self) -> str: ...
    @property
    def url(self) -> str: ...
    @property
    def headers(self) -> HttpHeaders: ...
    @property
    def data(self) -> str | bytes: ...

class HttpServer:
    def __init__(self, host: str, port: int, /) -> None: ...
    def start(self) -> ErrorCode: ...
    def stop(self) -> ErrorCode: ...
    def dispatch[T](self, fn: Callable[
        [HttpRequest],
        T | tuple[T, HttpStatusCode] | tuple[T, HttpStatusCode, HttpHeaders]
        ], /) -> bool:
        """Dispatch one HTTP request through `fn`. `fn` should return one of the following:

        + object
        + (object, status_code)
        + (object, status_code, headers)
        
        Return `True` if dispatched, otherwise `False`.
        """

    def ws_set_ping_interval(self, milliseconds: int, /) -> None:
        """Set WebSocket ping interval in milliseconds."""

    def ws_close(self, channel: WsChannelId, /) -> ErrorCode:
        """Close WebSocket channel."""

    def ws_send(self, channel: WsChannelId, data: str, /) -> int:
        """Send WebSocket message through `channel`."""

    def ws_recv(self) -> Union[
        tuple[Literal['onopen'], tuple[WsChannelId, HttpRequest]],
        tuple[Literal['onmessage'], tuple[WsChannelId, str]],
        tuple[Literal['onclose'], WsChannelId],
        None
    ]:
        """Receive one WebSocket message.
        Return one of the following or `None` if nothing to receive.
        
        + `"onopen"`: (channel, request)
        + `"onclose"`: channel
        + `"onmessage"`: (channel, body)
        """

class WebSocketClient:
    def open(self, url: str, headers=None, /) -> ErrorCode: ...
    def close(self) -> ErrorCode: ...

    def send(self, data: str, /) -> int:
        """Send WebSocket message."""

    def recv(self) -> Union[
        tuple[Literal['onopen'], None],
        tuple[Literal['onclose'], None],
        tuple[Literal['onmessage'], str],
        None
    ]:
        """Receive one WebSocket message.
        Return one of the following or `None` if nothing to receive.
        
        + `"onopen"`: `None`
        + `"onclose"`: `None`
        + `"onmessage"`: body
        """


def strerror(errno: ErrorCode, /) -> str:
    """Get error message by errno via `hv_strerror`."""

