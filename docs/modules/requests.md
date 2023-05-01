---
icon: package
label: requests
---

!!!
This module is experimental. To enable it, download `httplib.h` from [here](https://github.com/yhirose/cpp-httplib) and place it in the same directory as `pocketpy.h`.

SSL is not supported.
!!!

### `requests.get(url, headers=None) -> Response`

Send a GET request to `url` and return a `Response` object.

### `requests.post(url, data=None, headers=None) -> Response`

Send a POST request to `url` and return a `Response` object.

### `requests.put(url, data=None, headers=None) -> Response`

Send a PUT request to `url` and return a `Response` object.

### `requests.delete(url, headers=None) -> Response`

Send a DELETE request to `url` and return a `Response` object.