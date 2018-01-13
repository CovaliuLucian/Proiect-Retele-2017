//
// Created by lucian on 13.01.2018.
//

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMDocument.hpp>

#include <iostream>


using namespace std;
using namespace xercesc;

void main()
{
    XMLPlatformUtils::Initialize();

    p_DOMDocument = p_DOMImplementation->createDocument(0, L"ad", 0);




    XMLPlatformUtils::Terminate();
}

