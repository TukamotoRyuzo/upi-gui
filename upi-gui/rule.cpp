#include "rule.h"
#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>

template <typename T>
void getXMLValue(boost::property_tree::ptree& pt, std::string key, T& ref) {
    boost::optional<T> value = pt.get_optional<T>(key);
    if (value) {
        ref = value.get();
        std::cout << ref << std::endl;
    }
    else {
        throw std::runtime_error(("no such property: " + key).c_str());
    }
};

void Rule::load() {
    try {
        boost::property_tree::ptree pt;
        read_xml("rule.xml", pt);
        getXMLValue(pt, "Root.Rule.ChainTime", chain_time);
        getXMLValue(pt, "Root.Rule.NextTime", next_time);
        getXMLValue(pt, "Root.Rule.SetTime", set_time);
        getXMLValue(pt, "Root.Rule.FallTime", fall_time);
        getXMLValue(pt, "Root.Rule.AutoDropTime", autodrop_time);
        getXMLValue(pt, "Root.Setting.Continuous", continuous);
        getXMLValue(pt, "Root.Setting.Voice", voice);
        getXMLValue(pt, "Root.Setting.ai_1p", ai_1p);
        getXMLValue(pt, "Root.Setting.ai_2p", ai_2p);
        getXMLValue(pt, "Root.Setting.engine_name_1p", engine_name_1p);
        getXMLValue(pt, "Root.Setting.engine_name_2p", engine_name_2p);
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }
}

void Rule::save() {
    boost::property_tree::ptree pt;
    pt.put("Root.Rule.ChainTime", chain_time);
    pt.put("Root.Rule.NextTime", next_time);
    pt.put("Root.Rule.SetTime", set_time);
    pt.put("Root.Rule.FallTime", fall_time);
    pt.put("Root.Rule.AutoDropTime", autodrop_time);
    pt.put("Root.Setting.Continuous", continuous);
    pt.put("Root.Setting.Voice", voice);
    pt.put("Root.Setting.ai_1p", ai_1p);
    pt.put("Root.Setting.ai_2p", ai_2p);
    pt.put("Root.Setting.engine_name_1p", engine_name_1p);
    pt.put("Root.Setting.engine_name_2p", engine_name_2p);

    const int indent = 2;
    write_xml("rule.xml", pt, std::locale(),
        boost::property_tree::xml_writer_make_settings<std::string>(' ', indent));
}
