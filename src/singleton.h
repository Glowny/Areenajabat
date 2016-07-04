#pragma once

#define MAKE_SINGLETON(__typename__) static __typename__& instance() \
									 { \
											static __typename__ inst; \
											return inst; \
									 } \
									 __typename__(__typename__ const& copy) = delete; \
									 __typename__& operator=(__typename__ const& copy) = delete; \
									 protected: \
										__typename__(); \
									 public: 

#define MAKE_SINGLETON_NO_CTOR(__typename__) static __typename__& instance() \
									 { \
											static __typename__ inst; \
											return inst; \
									 } \
									 __typename__(__typename__ const& copy) = delete; \
									 __typename__& operator=(__typename__ const& copy) = delete; 
