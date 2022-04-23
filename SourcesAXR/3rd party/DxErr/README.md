# DxErr
One of the little utility libraries in the DirectX SDK is a static library for converting HRESULTs
to text strings for debugging and diagnostics known as DXERR.LIB. There were once even older versions
of this library, DXERR8.LIB and DXERR9.LIB, but they were removed from the DirectX SDK many years back
in favor of a unified DXERR.LIB.

However, this library was removed from Windows SDK 8.0
(see "[Where is DXERR.LIB?](http://blogs.msdn.com/b/chuckw/archive/2012/04/24/where-s-dxerr-lib.aspx)").

DXERR.LIB contained the following functions (both ASCII and UNICODE):

* `DXGetErrorString`
* `DXGetErrorDescription`
* `DXTrace`

And the macros `DXTRACE_MSG`, `DXTRACE_ERR`, `DXTRACE_ERR_MSGBOX`

This package was taken from
[Chuck Walbourn's MSDN blog](http://blogs.msdn.com/b/chuckw/archive/2012/04/24/where-s-dxerr-lib.aspx)
and modified to comply with software that uses ANSI encoding.
