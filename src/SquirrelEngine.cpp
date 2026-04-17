
#include "SquirrelEngine.h"

#ifndef NO_STB_LIBS
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#endif

#endif
#endif // NO_STB_LIBS

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <iconv.h>
#include <wchar.h>
#include <ctype.h> // isprint

#ifndef NO_STB_LIBS
void SaveBMData(const char *fp, int w, int h, int bpp, void *data)
{
    stbi_write_bmp(fp, w, h, bpp, data);
}
#endif

static int HasNonPrintableChar(const char* data, int len)
{
    for (int i = 0; i < len; i++)
    {
	unsigned char c = data[i];
	if (isprint(c) == 0) return 1;
    }
    return 0;
}


static void Bin2HexStr(const uint8_t *bin, size_t binLen, char *outBuf)
{
    if (bin && outBuf)
    {
	char *outBufPtr = outBuf;
	for(int i=0 ; i < binLen; i++)
	{
	    outBufPtr += sprintf(outBufPtr, "\\x%02hhx", bin[i]); // ++append
	}	 
    }
}

#ifdef __SYMBIAN32__
#include <e32std.h>

static void Bin2HexWStr(const uint8_t *bin, size_t binLen, const wchar_t *outBuf, size_t outLen)
{
    if (bin && outBuf)
    {
	_LIT(KHex, "\\x%02x");
	TPtr16 p((TUint16*)outBuf, outLen/2);
	for(int i=0 ; i < binLen; i++)
	{
	    p.AppendFormat(KHex, bin[i]);
	}
	p.ZeroTerminate();
    }
}

#else 

static void Bin2HexWStr(const uint8_t *bin, size_t binLen, const wchar_t *outBuf, size_t outLen)
{
    //int resLen = 0;
    if (bin && outBuf)
    {
	wchar_t *outBufPtr = (wchar_t*)outBuf;
	for(int i=0 ; i < binLen; i++)
	{

	    outBufPtr += swprintf(outBufPtr, 5, L"\\x%02x", bin[i]);// ++append

	}	 
    }
    //return outLen;
}
#endif 

QRBARDecoder::QRBARDecoder()
{
    m_scanner = NULL;
    m_image = NULL;
    m_imgbuf = NULL;
    m_cachedPayload = NULL;
    m_cachedPayloadSize = 0;
    m_imgbufLen = 0;   
    SetError((const char*)NULL);
}

QRBARDecoder::~QRBARDecoder()
{
    if (m_image){
	zbar_image_destroy(m_image);
	m_image = NULL;
    }

    if (m_scanner){
	zbar_image_scanner_destroy(m_scanner);
	m_scanner = NULL;
    }

    if (m_cachedPayload) delete m_cachedPayload;
    m_cachedPayload = NULL;
#ifndef NO_STB_LIBS
    ReleaseImgBuf();
#endif
}

int QRBARDecoder::Init()
{

    if (m_scanner){
	zbar_image_scanner_destroy(m_scanner);
    }

    m_scanner = zbar_image_scanner_create();
    zbar_image_scanner_set_config(m_scanner, ZBAR_NONE, ZBAR_CFG_BINARY, 1);
    zbar_image_scanner_set_config(m_scanner, ZBAR_NONE, ZBAR_CFG_ENABLE, 1);
   
    if (!m_scanner)
    {
	SetError(strerror(ENOMEM)); 
	return 0;
    }

    if (m_image) zbar_image_destroy(m_image);

    m_image = zbar_image_create();
    if (!m_image)
    {
	SetError(strerror(ENOMEM)); 
	return 0;
    }

    zbar_image_set_format(m_image, zbar_fourcc('Y', '8', '0', '0'));
    return 1;
}

int QRBARDecoder::Init(int w, int h)
{
    if (Init())
    {
	if (!m_image){
	    m_image = zbar_image_create();
	    zbar_image_set_format(m_image, zbar_fourcc('Y', '8', '0', '0'));
	}
	zbar_image_set_size(m_image, w, h);
    }

    if (!m_image || !m_scanner)
    {
	SetError(strerror(ENOMEM));
	return 0;
    }
    return 1;
}


int QRBARDecoder::DecodeImageData(void* data, size_t len)
{

    if (!m_scanner || !m_image){
	SetError(strerror(EINVAL));
	return -1;
    }
    
    zbar_image_set_data(m_image, data, len, NULL);
    return zbar_scan_image(m_scanner, m_image);
}


int QRBARDecoder::DecodeImageData(void* data, int w, int h)
{
    if (!m_scanner || !m_image)
    {
	if (!Init(w, h)) return -1;
    }
    else 
    {
	zbar_image_set_size(m_image, w, h);
    }

    //SaveBMData("D:\\1bpp.bmp", w, h, 1, data);
    return DecodeImageData(data, w*h);
}

#ifndef NO_STB_LIBS

void QRBARDecoder::ReleaseImgBuf()
{
    if (m_imgbuf) stbi_image_free(m_imgbuf);
    m_imgbuf = NULL;
}

int QRBARDecoder::DecodeImageData(void* data, int w, int h, int bpp)
{

    size_t len = w*h;
    if (m_imgbufLen != len)
    {
	ReleaseImgBuf();
	m_imgbuf = (uint8_t*) malloc(len);
	if (!m_imgbuf)
	{
	    SetError(strerror(ENOMEM));
	    return -1;
	}
    }

    m_imgbufLen = len;
    stbi_convert_format(m_imgbuf, (stbi_uc*)data, bpp, 1, w, h);

    //SaveBMData("D:\\1bpp.bmp", w, h, 1, m_imgbuf);
    return DecodeImageData(m_imgbuf, w, h);
}

int QRBARDecoder::DecodeImage(const char* fp)
{
    int w, h, bpp;

    ReleaseImgBuf();
    m_imgbuf = stbi_load(fp, &w, &h, &bpp, 1);

    if (!m_imgbuf)
    {
	SetError(stbi_failure_reason());
	return -1;
    }

    return DecodeImageData(m_imgbuf, w, h);
}


int QRBARDecoder::DecodeImage(const wchar_t *fp)
{
    int w, h, bpp;
    FILE* f;

    if (!(f = wfopen(fp, L"rb")))
    {
	SetError(strerror(errno));
	return 0;
    }

    ReleaseImgBuf();
    m_imgbuf = stbi_load_from_file(f, &w, &h, &bpp, 1);

    if (!m_imgbuf)
    {
	SetError(stbi_failure_reason());
	return -1;
    }

    fclose(f);
    return DecodeImageData(m_imgbuf, w, h);
}
#endif // NO_STB_LIBS

void QRBARDecoder::GetCodePosition(const zbar_symbol_t *symbol, DecoderResult* res)
{
    unsigned int loc_count = zbar_symbol_get_loc_size(symbol);
    loc_count = loc_count >= 4 ? 4 : 0;
    for (unsigned int i = 0; i < loc_count; i++)
    {
	int x = zbar_symbol_get_loc_x(symbol, i); 
	int y = zbar_symbol_get_loc_y(symbol, i);
	res->code_pos[i][0] = x;
	res->code_pos[i][1] = y;
    }
}

int QRBARDecoder::ExtractCode(void* data, int w, int h, DecoderResult* res)
{
    if (!m_scanner || !m_image)
    {
	if (!Init(w, h)) return -1;
    }
    else 
    {
	zbar_image_set_size(m_image, w, h);
    }

    zbar_image_set_data(m_image, data, w*h, NULL);
    zbar_image_scanner_set_config(m_scanner, ZBAR_NONE, ZBAR_CFG_BINARY, 1);


    int syms = zbar_scan_image(m_scanner, m_image);
    res->has_qrcode = 0;  
    res->payload_type = PL_TEXT;

    if (syms > 0)
    {
	const zbar_symbol_t *symbol = zbar_image_first_symbol(m_image);
	
	if (!symbol) goto err;

	res->has_qrcode = (zbar_symbol_get_type(symbol) == ZBAR_QRCODE);

	const char* bin = zbar_symbol_get_data(symbol);
	int bin_len = zbar_symbol_get_data_length(symbol);
	
	if (res->has_qrcode) GetCodePosition(symbol, res);

	// HACK: rescan in order to detect if the payload is binary or UTF-8 text.
	zbar_image_scanner_set_config(m_scanner, ZBAR_NONE, ZBAR_CFG_BINARY, 0);
	if (zbar_scan_image(m_scanner, m_image) < 1) 
	{
	    // no UTF-8 ... iconv failed 
	    if (res->has_qrcode)
	    {
		res->payload_type = PL_BINARY;
		res->payload = bin;
		res->payload_len = bin_len;
		return syms;
	    }
	    else{
		goto err;
	    }
	}

	symbol = zbar_image_first_symbol(m_image);	
	if (!symbol) goto err;
 
	const char* text = zbar_symbol_get_data(symbol);
	int text_len = zbar_symbol_get_data_length(symbol);
	

	res->payload_len = text_len;
	res->payload = text;
	if (res->has_qrcode)
	{
	    // TODO: check for valid UTF-8?
	    if ( (text_len != bin_len) && HasNonPrintableChar(bin, bin_len) )
	    {
		res->payload_type = PL_BINARY;
		res->payload = bin;
		res->payload_len = bin_len;
	    }

	}

    }

    else if (syms == -1) goto err;

    return syms;

    err:    
	SetError(strerror(errno));
	return -1;
    
}



int QRBARDecoder::ExtractCode(int index, DecoderResult *res)
{
    if (!m_image || !m_scanner)
    {
	SetError(strerror(EINVAL));
	return 0;
    }
    
    const zbar_symbol_t *symbol = zbar_image_first_symbol(m_image);
    
    res->has_qrcode = 0;
    res->payload_type = PL_TEXT;
    
    int i = 0;
    for (; symbol; symbol = zbar_symbol_next(symbol)) {
	if (i == index)
	{
	    res->has_qrcode = ( zbar_symbol_get_type(symbol) == ZBAR_QRCODE);

	    res->payload_len = zbar_symbol_get_data_length(symbol);

	    res->payload = zbar_symbol_get_data(symbol);
	    break;
	}
	i++;
    }

    return 1;
}



int QRBARDecoder::AllocateCachedPayload(const int len)
{
    if (m_cachedPayload)
    {
	delete m_cachedPayload;
	m_cachedPayload = NULL;
	m_cachedPayloadSize = 0;
    }

    m_cachedPayload = new char[len];
    if(!m_cachedPayload)
    {
	SetError(strerror(errno));
	return 0;
    }
    m_cachedPayloadSize = len;
    return 1;
}

int QRBARDecoder::ConvertPayloadData(const char *inEnc, const char *outEnc, char *inBuf, size_t inBufLen)
{
    int len;

    if (!m_cachedPayload) return 0;

    iconv_t cd = iconv_open(outEnc, inEnc);
    if(cd == (iconv_t)-1)
    {
	SetError(strerror(errno));
	return 0;
    }

    memset(m_cachedPayload, 0, m_cachedPayloadSize);

    const char* inBufPtr = (const char*) inBuf;
    char* outBufPtr = m_cachedPayload;
    size_t leftBytes = m_cachedPayloadSize;

    size_t nc = iconv(cd, &inBufPtr, &inBufLen, &outBufPtr, &leftBytes);
    if (nc == (size_t)-1)
    {
	SetError(strerror(errno));
	len = 0;
    }

    else
    {
	len = m_cachedPayloadSize - leftBytes;
    }

    iconv_close(cd);
    return len;
}

#define TRY_CONVERT_PAYLOAD(inEnc) do { \
	int ret = ConvertPayloadData(inEnc, outEnc, payload, payloadLen); \
	if (ret){ \
	    outLen = ret; \
	    return (const char*) m_cachedPayload;} \
    } while(0);

#define PAYLOAD2HEX() do { \
	if (!AllocateCachedPayload((payloadLen + 1)*10)) return NULL; \
	memset(m_cachedPayload, 0, m_cachedPayloadSize); \
	const wchar_t *wstr = (const wchar_t*) m_cachedPayload; \
	Bin2HexWStr((const uint8_t*) payload, payloadLen, wstr, m_cachedPayloadSize); \
	outLen = wcslen(wstr); \
	return (const char*) m_cachedPayload; \
    } while(0);


#define MATCH_PREFIX(s1, s2, l) (strncmp(s1, s2, l) == 0)

#define MATCH_PREFIX2(s1, s2, l) (strncasecmp(s1, s2, l) == 0)

// s1 must must be longer than s2 (suffix)
#define MATCH_SUFFIX(s1, s2, l) (strncmp((s1) + (l - strlen(s2)), s2, strlen(s2)) == 0)

struct PLTypeCheck {
    const char* prefix;
    const char* suffix;
    PLType t;
};

#define URL_TC(p) {p, NULL, PL_URL}

void QRBARDecoder::ParsePayloadType(DecoderResult  *qrc)
{
    
    size_t payloadLen = (size_t) qrc->payload_len;
    const char* payload = qrc->payload;
    qrc->payload_type = PL_TEXT;

    static PLTypeCheck typeChecks[] = {
	
	/*{"MECARD:", NULL, PL_CONTACT},*/
	{"BEGIN:VCARD", "END:VCARD", PL_CONTACT},	
	URL_TC("http://"),
	URL_TC("https://"),
	URL_TC("ftp://"),
	URL_TC("ftps://"),
	URL_TC("file://"),
	{"BEGIN:VCALENDAR", NULL/*"END:VCALENDAR"*/, PL_CALENDAR},
	{"mailto:", NULL, PL_EMAIL},
	{"tel:", NULL, PL_PHONE_NUMBER},
	{"geo:", NULL, PL_GEO_LOCATION},
	{"sms:", NULL, PL_SMS},
	{"WIFI:", NULL, PL_WIFI}
    };

    const int count = sizeof(typeChecks) / sizeof(PLTypeCheck);

    for (int i = 0; i < count; i++)
    {
	PLTypeCheck tc = typeChecks[i];
	const char* p = tc.prefix;
	const char* s = tc.suffix;
	size_t pLen = (p ? strlen(p) : 0);
	size_t sLen = (s ? strlen(s) : 0);

	if ((payloadLen >= pLen) && MATCH_PREFIX(payload, p, pLen))
	{
	    if (s && (payloadLen >= sLen) && !MATCH_SUFFIX(payload, s, sLen)) continue;

	    qrc->payload_type = tc.t;
	    break;
	}
    }
}

const char* QRBARDecoder::EncodePayload(DecoderResult* qrc, int &outLen, const char* outEnc)
{
    size_t payloadLen = (size_t) qrc->payload_len;
    char* payload = (char*) qrc->payload;

    if (!AllocateCachedPayload((payloadLen + 1)*4)) return NULL;

    TRY_CONVERT_PAYLOAD("UTF-8")
    return NULL;
}


const char* QRBARDecoder::ParseAndEncodePayload(DecoderResult *qrc, int &outLen, const char* outEnc)
{
    size_t payloadLen = (size_t) qrc->payload_len;
    char* payload = (char*) qrc->payload;
    
    if (!AllocateCachedPayload((payloadLen + 1)*4)) return NULL;

    if (qrc->payload_type == PL_BINARY)
    {
	PAYLOAD2HEX()
    }

    else
    {
	ParsePayloadType(qrc);
	TRY_CONVERT_PAYLOAD("UTF-8");
    }
    
    return NULL;
}


void QRBARDecoder::SetError(const char* error)
{
    memset(m_error, 0, sizeof(m_error));   
    if (error) strncpy(m_error, error, sizeof(m_error)-1);
}

void QRBARDecoder::SetError(char* error)
{
    SetError((const char*)error);
}

const char* QRBARDecoder::GetError()
{
    if (strlen(m_error) == 0) return NULL;
    return m_error;
}

const wchar_t* QRBARDecoder::GetWError()
{
    const char* error = GetError();
    if (!error) return NULL;

    static wchar_t werror[512];
    memset(werror, 0, sizeof(werror));

    if (mbstowcs(werror, error, strlen(error)) == (size_t)-1)
    {
	return NULL;
    }
    return werror;
}



uint8_t QRCEncoder::QrcBuf[qrcodegen_BUFFER_LEN_MAX];

QRCEncoder::QRCEncoder()
{
    SetParams(qrcodegen_Ecc_HIGH, qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX); 
}

void QRCEncoder::SetParams(int ecl, int minVersion, int maxVersion)
{
    m_qrcSize = 0;
    m_ecl = ecl;
    m_minVersion = minVersion;
    m_maxVersion = maxVersion;
}

int QRCEncoder::EncodeText(const char *text)
{

    uint8_t tempBuf[qrcodegen_BUFFER_LEN_MAX] = {0};

    bool ok = qrcodegen_encodeText(text, tempBuf, QRCEncoder::QrcBuf, static_cast<qrcodegen_Ecc>(m_ecl), m_minVersion, m_maxVersion, qrcodegen_Mask_AUTO, true);
   
    m_qrcSize = ok ? qrcodegen_getSize(QRCEncoder::QrcBuf) :0;
    return m_qrcSize;
}

int QRCEncoder::GetModule(int x, int y)
{
    if (m_qrcSize == 0) return -1;
    return qrcodegen_getModule(QRCEncoder::QrcBuf, x, y);
}

