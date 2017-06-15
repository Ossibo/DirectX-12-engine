#ifndef _DIRECTX_H_
#define _DIRECTX_H_

#define SAFE_RELEASE(x) if( x ) { (x)->Release(); (x) = 0; }
#define SAFE_RELEASE_INPUT(x) if( x ) { (x)->Unacquire(); (x)->Release(); (x) = 0; }
#define SAFE_DELETE(x) if( x ) { delete(x); (x) = 0; }
#define SAFE_DELETE_ARRAY(x) if( x ) { delete[](x); (x) = 0; }
#define ASSERT(r) if (!r) { return false; }		// BOOL
#define FAIL(r) if(FAILED(r)) { return false; }	// HRESULT

#endif