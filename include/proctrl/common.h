//
// Created by edward on 2016/7/17.
//

#ifndef PROCTRL_COMMON_H
#define PROCTRL_COMMON_H
#include <string>
#include <vector>
#include <set>
#include <map>
#include <utility>	/* pair */
#include <string>
#include <fstream>
#include <boost/unordered_map.hpp>

#ifndef Q_MOC_RUN
#include <boost/serialization/set.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
/* XXX: binary serialization may be a little faster... */
#ifdef USE_BINARY_ARCHIVE
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#else
#include <boost/serialization/nvp.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#endif
#endif


#endif //PROCTRL_COMMON_H
