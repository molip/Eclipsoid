#include "Xml.h"
#include "App.h"
#include "tinyxml.h"

namespace Xml
{

Document::Document()
{
	m_pDoc.reset(new TiXmlDocument);
}

Document::~Document() {}

std::string Document::SaveToString() const
{
	TiXmlPrinter printer;
	m_pDoc->Accept(&printer);
	return printer.Str();
}

bool Document::LoadFromString(const std::string str) const
{
	m_pDoc->Parse(str.c_str());
	return !m_pDoc->Error();
}

bool Document::LoadFromFile(const std::string& path)
{
	return m_pDoc->LoadFile(path);
}

bool Document::SaveToFile(const std::string& path) const
{
	return m_pDoc->SaveFile(path);
}

Element Document::AddElement(const std::string& name)
{
	TiXmlElement* pElem = new TiXmlElement(name);
	m_pDoc->LinkEndChild(pElem);
	return Element(pElem);
}

Element Document::GetRoot()
{
	auto pRoot = m_pDoc->RootElement();
	VERIFY_SERIAL(!!pRoot);
	return Element(pRoot);
}

//-----------------------------------------------------------------------------

Element::Element(TiXmlElement* pElem) : m_pElem(pElem)
{
}

const std::string& Element::GetName() const
{
	VERIFY_SERIAL(!!m_pElem);
	return m_pElem->ValueStr();
}

Element Element::AddElement(const std::string& name)
{
	VERIFY_SERIAL(!!m_pElem);
	TiXmlElement* pElem = new TiXmlElement(name);
	m_pElem->LinkEndChild(pElem);
	return Element(pElem);
}

void Element::SetAttribute(const std::string& name, const std::string& val)
{
	VERIFY_SERIAL(!!m_pElem);
	m_pElem->SetAttribute(name, val);
}

void Element::SetAttribute(const std::string& name, int val)
{
	VERIFY_SERIAL(!!m_pElem);
	m_pElem->SetAttribute(name, val);
}

bool Element::GetAttribute(const std::string& name, std::string& val) const
{
	VERIFY_SERIAL(!!m_pElem);
	if (auto pStr = m_pElem->Attribute(name))
	{
		val = *pStr;
		return true;
	}
	return false;
}

bool Element::GetAttribute(const std::string& name, int& val) const
{
	VERIFY_SERIAL(!!m_pElem);
	return !!m_pElem->Attribute(name, &val);
}

bool Element::GetAttribute(const std::string& name, bool& val) const
{
	VERIFY_SERIAL(!!m_pElem);
	return m_pElem->QueryBoolAttribute(name.c_str(), &val) == TIXML_SUCCESS;
}

const std::string& Element::GetAttributeStr(const std::string& name) const
{
	VERIFY_SERIAL(!!m_pElem);
	auto pStr = m_pElem->Attribute(name);
	VERIFY_SERIAL(!!pStr);
	return *pStr;
}

int Element::GetAttributeInt(const std::string& name) const
{
	VERIFY_SERIAL(!!m_pElem);
	int val = 0;
	auto pStr = m_pElem->Attribute(name, &val);
	VERIFY_SERIAL(!!pStr);
	return val;
}

bool Element::GetAttributeBool(const std::string& name) const
{
	VERIFY_SERIAL(!!m_pElem);
	bool val = false;
	bool bOK = m_pElem->QueryBoolAttribute(name.c_str(), &val) == TIXML_SUCCESS;
	VERIFY_SERIAL(bOK);
	return val;
}

Element Element::GetFirstChild(const std::string& name) const
{
	VERIFY_SERIAL(!!m_pElem);
	auto pEl = m_pElem->FirstChildElement(name);
	return Element(pEl);
}

Element Element::GetNextSibling(const std::string& name) const
{
	VERIFY_SERIAL(!!m_pElem);
	return Element(m_pElem->NextSiblingElement(name));
}

}
