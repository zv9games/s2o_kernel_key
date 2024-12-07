TARGETNAME=s2o_kernel_key
TARGETTYPE=DRIVER

INCLUDES=include
SOURCES=src\driver.c

all: $(TARGETNAME).sys

$(TARGETNAME).sys: $(SOURCES)
    cl /D WIN32 /D _WINDOWS /D DRIVER /D WDM_DRIVER /D WPP_CONTROL_GUIDS_DEFINED /D _AMD64_ /D _X64_ /I $(INCLUDES) /Zi /W4 /Od /Oy- /Zp8 /GS /Gy /fp:precise /Qspectre /Zc:wchar_t- /Zc:forScope /Zc:inline /Fo"x64\Release\\" /Fe"x64\Release\$(TARGETNAME).sys" $(SOURCES)
