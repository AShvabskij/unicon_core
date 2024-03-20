TEMPLATE = subdirs

unix: SUBDIRS += dde_lib/utils/IPCmemLib/ipcmem_lib.pro

SUBDIRS += \
    dde_lib/DDE_lib.pro \
    dde_lib/utils/sql3_db \
#   unicon_testpanel \
    unicon_core

  win32: SUBDIRS += gabbi_convertor


# unix: SUBDIRS += \
#    dde_lib/utils/parser
