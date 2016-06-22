#pragma once

#define FORWARD_DECLARE_TYPE_STRUCT			struct
#define FORWARD_DECLARE_TYPE_CLASS			class
#define FORWARD_DECLARE_TYPE_ENUM_CLASS		enum class

#define FORWARD_DECLARE(__fdtype__, __name__)								__fdtype__ __name__;
#define FORWARD_DECLARE_1(__fdtype__, __ns1__, __name__)					namespace __ns1__ { __fdtype__ __name__; }
#define FORWARD_DECLARE_2(__fdtype__, __ns1__, __ns2__, __name__)			namespace __ns1__ { namespace __ns2__ { __fdtype__ __name__; } } 
#define FORWARD_DECLARE_3(__fdtype__, __ns1__, __ns2__, __ns3__, __name__)	namespace __ns1__ { namespace __ns2__ { namespace __ns3__ { __fdtype__ __name__; } } }