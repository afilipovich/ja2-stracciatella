#include "Debug.h"
#include "Environment.h"
#include "FileMan.h"
#include "JAScreens.h"
#include "LoadSaveData.h"
#include "LoadSaveLightSprite.h"
#include "ScreenIDs.h"
#include "Sys_Globals.h"


void ExtractLightSprite(const BYTE** const data, const UINT32 light_time)
{
	INT16  x;
	INT16  y;
	UINT32 flags;
	UINT32 light_type;

	const BYTE* d = *data;
	EXTR_I16(d, x)
	EXTR_I16(d, y)
	EXTR_SKIP(d, 12)
	EXTR_U32(d, flags)
	EXTR_U32(d, light_type)
	Assert(d == *data + 24);

	UINT8 str_len;
	EXTR_U8(d, str_len)
	char template_name[str_len];
	EXTR_STR(d, template_name, str_len)
	template_name[str_len] = '\0';

	LIGHT_SPRITE* const l = LightSpriteCreate(template_name, light_type);
	// if this fails, then we will ignore the light.
	// ATE: Don't add ANY lights of mapscreen util is on
	if (l != NULL && guiCurrentScreen != MAPUTILITY_SCREEN)
	{
		// power only valid lights
		if (gfEditMode ||
				!gfCaves && (
					flags & light_time ||
					!(flags & (LIGHT_PRIMETIME | LIGHT_NIGHTTIME))
				))
		{
			LightSpritePower(l, TRUE);
		}
		LightSpritePosition(l, x, y);
		if (flags & LIGHT_PRIMETIME)
		{
			l->uiFlags |= LIGHT_PRIMETIME;
		}
		else if (flags & LIGHT_NIGHTTIME)
		{
			l->uiFlags |= LIGHT_NIGHTTIME;
		}
	}

	*data = d;
}


BOOLEAN InjectLightSpriteIntoFile(const HWFILE file, const LIGHT_SPRITE* const l)
{
	BYTE data[24];

	BYTE* d = data;
	INJ_I16(d, l->iX)
	INJ_I16(d, l->iY)
	INJ_SKIP(d, 12)
	INJ_U32(d, l->uiFlags)
	INJ_U32(d, l->uiLightType)
	Assert(d == endof(data));

	if (!FileWrite(file, data, sizeof(data))) return FALSE;

	const char* const light_name = LightSpriteGetTypeName(l);
	const UINT8       str_len    = strlen(light_name) + 1;
	return
		FileWrite(file, &str_len,   sizeof(str_len)) &&
		FileWrite(file, light_name, str_len);
}
