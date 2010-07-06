///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Michael A. Jackson. BlueQuartz Software
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
//
///////////////////////////////////////////////////////////////////////////////
#ifndef _MXASetGetMacros_h_
#define _MXASetGetMacros_h_

#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>


#if defined(QT_CORE_LIB)
//-- Qt includes
#include <QtCore/QSharedPointer>
#else
//-- Boost Includes
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#endif

#define SHARED_IS_NULL(ptr)\
  (  (ptr).get() == NULL )


/**
 * @brief Creates a static method that returns a NULL pointer wrapped in a
 * boost::shared_ptr<>
 * @param thisClass The name of the class.
 */
#define MXA_NULL_SHARED_POINTER(thisClass)\
  static Pointer NullPointer(void)\
  { \
    return Pointer(static_cast<thisClass*>(NULL));\
  }

#if defined(QT_CORE_LIB)
/**
 * @brief Creates some basic typedefs that can be used throughout the code to
 * reference the class.
 */
#define MXA_SHARED_POINTERS(thisClass)\
  typedef thisClass                      Self;\
  typedef QSharedPointer< Self >        Pointer;\
  typedef QSharedPointer<const Self >  ConstPointer;\
  MXA_NULL_SHARED_POINTER(thisClass)

#else
/**
 * @brief Creates some basic typedefs that can be used throughout the code to
 * reference the class.
 */
#define MXA_SHARED_POINTERS(thisClass)\
  typedef thisClass                      Self;\
  typedef boost::shared_ptr<Self >        Pointer;\
  typedef boost::shared_ptr<const Self >  ConstPointer;\
  typedef boost::weak_ptr<thisClass > WeakPointer;\
  typedef boost::weak_ptr<thisClass > ConstWeakPointer;\
  MXA_NULL_SHARED_POINTER(thisClass)

#endif

/**
 * @brief Creates a typedef that points to the superclass of this class
 */
#define MXA_SUPERCLASS_POINTER(SuperClass)\
  typedef SuperClass::Pointer SuperClass;


/**
 * @brief Creates a static "New" method that creates an instance of thisClass
 */
#define MXA_NEW_SUPERCLASS(thisClass, SuperClass)\
  typedef SuperClass::Pointer SuperClass##Type;\
  static SuperClass##Type New##SuperClass(void) \
{ \
  SuperClass##Type sharedPtr (new thisClass); \
  return sharedPtr; \
}

/**
 * @brief Implements a Static 'New' Method for a class
 */
#define MXA_STATIC_NEW_MACRO(thisClass) \
static Pointer New(void) \
{ \
  Pointer sharedPtr (new thisClass); \
  return sharedPtr; \
}

/** Macro used to add standard methods to all classes, mainly type
 * information. */
#define MXA_TYPE_MACRO(thisClass) \
    virtual const char* getNameOfClass() const \
        {return #thisClass;}

//------------------------------------------------------------------------------
// Macros for Properties
/**
* @brief Creates a std::string constant for the Property so that the property
* can be retrieved by name.
*/
#define MXA_PROPERTY_CONSTANT(prpty) \
  const std::string prpty ( #prpty );

/**
* @brief Creates a "setter" method to set the property.
*/
#define MXA_SET_PROPERTY(type, prpty, varname) \
  void set##prpty(type value) { varname = value; }

/**
* @brief Creates a "getter" method to retrieve the value of the property.
*/
#define MXA_GET_PROPERTY(type, prpty, varname) \
  type get##prpty() { return varname; }

/**
* @brief Convenience macro to create both the setter and getter methods.
*/
#define MXA_PROPERTY(type, prpty, varname) \
  MXA_SET_PROPERTY(type, prpty, varname)\
  MXA_GET_PROPERTY(type, prpty, varname)

/**
* @brief Convenience macro to create both the setter and getter methods in addition
* to the instance variable.
*/
#define MXA_INSTANCE_PROPERTY(type, prpty, varname)\
  private:\
      type   varname;\
  public:\
    MXA_SET_PROPERTY(type, prpty, varname)\
    MXA_GET_PROPERTY(type, prpty, varname)

#define MXA_VIRTUAL_INSTANCE_PROPERTY_m(type, prpty)\
  private:\
      type   m_##prpty;\
  public:\
    virtual MXA_SET_PROPERTY_m(type, prpty)\
    virtual MXA_GET_PROPERTY_m(type, prpty)

/**
* @brief Creates a "setter" method to set the property.
*/
#define MXA_SET_PROPERTY_m(type, prpty) \
  void set##prpty(type value) { this->m_##prpty = value; }

/**
* @brief Creates a "getter" method to retrieve the value of the property.
*/
#define MXA_GET_PROPERTY_m(type, prpty) \
  type get##prpty() { return m_##prpty; }


#define MXA_INSTANCE_PROPERTY_m(type, prpty)\
  private:\
      type   m_##prpty;\
  public:\
    MXA_SET_PROPERTY_m(type, prpty)\
    MXA_GET_PROPERTY_m(type, prpty)



#define MXA_SET_2DVECTOR_PROPERTY(type, prpty, varname)\
  void set##prpty(type value[2]) {\
      varname[0] = value[0]; varname[1] = value[1]; }\
  void set##prpty(type value_0, type value_1) {\
      varname[0] = value_0; varname[1] = value_1; }

#define MXA_GET_2DVECTOR_PROPERTY(type, prpty, varname)\
  void get##prpty(type value[2]) {\
      value[0] = varname[0]; value[1] = varname[1]; }\
  void get##prpty(type &value_0, type &value_1) {\
      value_0 = varname[0]; value_1 = varname[1]; }

#define MXA_INSTANCE_2DVECTOR_PROPERTY(type, prpty, varname)\
  private:\
    type   varname[2];\
  public:\
    MXA_SET_2DVECTOR_PROPERTY(type, prpty, varname)\
    MXA_GET_2DVECTOR_PROPERTY(type, prpty, varname)


/**
* @brief Creates a "setter" method to set the property.
*/
#define MXA_SET_STRING_PROPERTY( prpty, varname) \
  void set##prpty(const std::string &value) { this->varname = value; }

/**
* @brief Creates a "getter" method to retrieve the value of the property.
*/
#define MXA_GET_STRING_PROPERTY( prpty, varname) \
  std::string get##prpty() { return varname; }

/**
 * @brief Creates setters and getters in the form of 'setXXX()' and 'getXXX()' methods
 */
#define MXA_STRING_PROPERTY(prpty, varname)\
  MXA_SET_STRING_PROPERTY(prpty, varname)\
  MXA_GET_STRING_PROPERTY(prpty, varname)

#define MXA_INSTANCE_STRING_PROPERTY(prpty, varname)\
  private:\
  std::string      varname;\
  public:\
  MXA_SET_STRING_PROPERTY(prpty, varname)\
  MXA_GET_STRING_PROPERTY(prpty, varname)

#define MXA_INSTANCE_STRING_PROPERTY_m(prpty)\
  private:\
  std::string      m_##prpty;\
  public:\
  MXA_SET_STRING_PROPERTY(prpty,  m_##prpty)\
  MXA_GET_STRING_PROPERTY(prpty,  m_##prpty)

// These are simple over-rides from the boost distribution because we don't want the entire boost distribution just
// for a few boost headers
namespace MXA
{
  class bad_lexical_cast : public std::runtime_error {
  public:
    bad_lexical_cast(const std::string& s)
      : std::runtime_error(s)
    { }
  };

  class bad_any_cast : public std::runtime_error {
  public:
    bad_any_cast(const std::string& s)
      : std::runtime_error(s)
    { }
  };

  template<typename T>
  T lexical_cast(const std::string &s)
  {
    std::istringstream i(s);
    T x;
    if (!(i >> x))
      throw bad_lexical_cast("convertToDouble(\"" + s + "\")");

    return x;
  }
}


#if 1
/**
* @brief Creates an if conditional where the key is tested against the values constant
* and if a match found then the property value is set
* @param name_space The Namespace for the constants
* @param type The primitive type of the variable
* @param prpty the name of the property, usually with CamelCase spelling
* @param key The Key used for the property
* @param value The value of the property
*/
#define SET_PROPERTY_BODY(name_space, type, prpty, key, value) \
  if (name_space::prpty.compare(key) == 0) { \
    try { \
      this->set##prpty(MXA::lexical_cast<type>(value)); return 1; \
    }  catch(MXA::bad_lexical_cast &excp) { \
      std::cout << excp.what() << std::endl; \
      std::cout << "Could not convert value '" << value << "' to type '" << #type << "' for property '" << #prpty << "'" << std::endl; \
    } \
  }

#define SET_STRING_PROPERTY_BODY(name_space, type, prpty, key, value) \
  if (name_space::prpty.compare(key) == 0) { \
  try { \
  this->set##prpty(value); return 1; \
}  catch(MXA::bad_lexical_cast &excp) { \
  std::cout << excp.what() << std::endl; \
  std::cout << "Could not convert value '" << value << "' to type '" << #type << "' for property '" << #prpty << "'" << std::endl; \
} \
  }

/**
* @brief Creates an if condition where the key is tested against the values constant
* and if a match found the the property is returned using the 'value' argument
* @param name_space The Namespace for the constants
* @param type The primitive type of the variable
* @param prpty the name of the property, usually with CamelCase spelling
* @param varname The internal variable name used to store the property
* @param key The Key used for the property
* @param value The value of the property
*/

#define GET_PROPERTY_BODY(name_space, type, prpty, varname, key, value)\
  if (name_space::prpty.compare(key) == 0) {  \
  try { value = *(reinterpret_cast<T*>( &(varname))); return 1;} \
  catch(MXA::bad_any_cast &) { std::cout << "Could not cast value '" << value << "' to type '" << #type << "' for property '" << #prpty << "'" << std::endl; } }

#define GET_STRING_PROPERTY_BODY2(name_space, type, prpty, varname, key, value)\
  if (name_space::prpty.compare(key) == 0) {  \
  try { value = varname; return 1;} \
  catch(MXA::bad_any_cast &) { std::cout << "Could not cast value '" << value << "' to type '" << #type << "' for property '" << #prpty << "'" << std::endl; } }


//
////////////////////////////////////////////////////////////////////////////////

#endif




#endif /* _MXASetGetMacros_h_  */