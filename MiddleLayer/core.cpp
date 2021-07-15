#include "core.h"

#include <iostream>

#include "../DDE_CAN_LIB/DDE_CAN.h"

#include "requestmanager.h"
#include "responsemanager.h"
#include "paramshandler.h"
#include "devicehandler.h"

Core::Core()
{

}

Core::~Core()
{
    delete m_cmdServer;
}

void Core::start()
{
    test();

    ParamsHandler* params = new ParamsHandler();
    DeviceHandler* device = new DeviceHandler();

    RequestManager::instance()->registerHandler(device);
    RequestManager::instance()->registerHandler(params);
    ResponseManager::instance()->registerHandler(device);
    ResponseManager::instance()->registerHandler(params);
    StreamManager::instance()->registerHandler(params);

    m_cmdServer = new SocketServer(1235);
    m_cmdServer->setRequestManager(RequestManager::instance());
    m_cmdServer->setResponseManager(ResponseManager::instance());
    m_cmdServer->start();

    m_streamServer = new SocketServer(1237);
    m_streamServer->setRequestManager(RequestManager::instance());
    m_streamServer->setResponseManager(StreamManager::instance());
    m_streamServer->start();
}

int Core::test()
{
    std::cout << "DDE template started..." << std::endl;


    m_dde = new DDE_CAN();

    m_dde->init(0); //run thread

    //DDE_GET_PARAMS_HEADER s;
    //DDE_GET_EVLOG_HEADER evlog_header;
    //DDE_GET_EVLOG_DATA evlog_data;

    //for (int ii = 0; ii < 16; ii++) {
    //	evlog_header.device_ID = ii;
    //	m_dde->get_evlog_header(evlog_header);
    //	printf("device_ID=%d,  param = %d \n", evlog_header.device_ID, evlog_header.evlog_param1);
    //	if (evlog_header.device_ID > 0)
    //	{
    //		evlog_data.device_ID = evlog_header.device_ID;
    //		m_dde->get_evlog_data(evlog_data);
    //		printf("	msg_num=%d\n", evlog_data.msg_num);
    //		for (int ii = 0; ii < evlog_data.msg_num; ii++)
    //			printf("		msg_source=%d, msg_code=%d\n", evlog_data.msg[ii].source_ID, evlog_data.msg[ii].code_ID);
    //	}
    //
    //}


    //build params tree
    DDE_GET_PARAMS_HEADER get_devices_header;
    get_devices_header.device_ID = 0;
    m_dde->get_params_header(get_devices_header);

    std::cout << "HEADER el_count =" << get_devices_header.el_count << std::endl;

    for (int ii = 0; ii < get_devices_header.el_count; ii++) {
        std::string s(get_devices_header.el_descr[ii].name);
        std::cout << "	device name - " << s << " addr ="<< get_devices_header.el_descr[ii].id<<std::endl;

        ////try to get modules from device
        DDE_GET_PARAMS_HEADER get_modules_header;
        get_modules_header.elem_ID = 0;
        get_modules_header.device_ID = get_devices_header.el_descr[ii].id;
        m_dde->get_params_header(get_modules_header);
        print_modules(get_modules_header);

    }

    delete m_dde;

    return 0;
}

void Core::print_modules(const DDE_GET_PARAMS_HEADER& p)
{
        std::cout << "		modules_count =" << p.el_count << std::endl;

    for (int ii = 0; ii < p.el_count; ii++)
    {
        std::string s(p.el_descr[ii].name);
        std::cout << ii<<":                 module[" << p.el_descr[ii].id << "]  name = " << s << std::endl;


            DDE_GET_PARAMS_HEADER get_params_header;
            get_params_header.device_ID = p.device_ID;
            get_params_header.elem_ID = p.el_descr[ii].id;
            m_dde->get_params_header(get_params_header);
            print_params(p.device_ID,ii,get_params_header);
    }
    std::cout << std::endl;
}

void Core::print_params(int device_ID, int module_ID, const DDE_GET_PARAMS_HEADER& p)
{
    std::cout << "HEADER params count =" << p.el_count << "for device id = " << device_ID << ", module id = " << module_ID << std::endl;

    for (int ii = 0; ii < p.el_count; ii++)
    {
        std::string s(p.el_descr[ii].name);
        std::cout << ii << ":							param[" << p.el_descr[ii].id << "]  name = " << s << std::endl;
    }
    std::cout << std::endl;
}
