#include "rule.h"
#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>

void Rule::load() {
    try {
        boost::property_tree::ptree pt;
        read_xml("rule.xml", pt);

        auto getXMLValue = [&pt](std::string key, int& value) {
            boost::optional<std::string> str = pt.get_optional<std::string>(key);
            if (str) {
                value = std::stoi(str.get());
            }
            else {
                throw std::runtime_error(("no such property: " + str.get()).c_str());
            }
        };

        getXMLValue("Rule.ChainTime", chain_time);
        getXMLValue("Rule.NextTime", next_time);
        getXMLValue("Rule.SetTime", set_time);
        getXMLValue("Rule.FallTime", fall_time);
        getXMLValue("Rule.AutoDropTime", autodrop_time);
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }
}

void Rule::save() {
    boost::property_tree::ptree pt;
    boost::property_tree::ptree& child = pt.add("Rule", "");
    child.put("ChainTime", this->chain_time);
    child.put("NextTime", this->next_time);
    child.put("SetTime", this->set_time);
    child.put("FallTime", this->fall_time);
    child.put("AutoDropTime", this->autodrop_time);

    const int indent = 2;
    write_xml("rule.xml", pt, std::locale(),
        boost::property_tree::xml_writer_make_settings<std::string>(' ', indent));
}
