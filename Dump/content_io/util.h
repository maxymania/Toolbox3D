#pragma once

namespace content_io {
	namespace rc {
		template<class T>
		inline void destroy(T &p){
			if(p)p->drop();
			p = 0;
		}
		template<class T>
		inline void retain(T &p){
			if(p)p->grab();
		}
	}
}