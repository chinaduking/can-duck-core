//
// Created by sdong on 2020/11/12.
//
#ifndef NAMESPACE_CLASSNAME_HPP
#define NAMESPACE_CLASSNAME_HPP

/**
 * TODO_one_line_description_of_class.
 *
 * TODO_longer_description_of_class_meant_for_users._Developer_details_should_
 * be_put_in_the_.cpp_implementation
 *
 * @author  TODO_name, TODO_organization
 * @date    TODO_dd_mmm_yyyy
 */
class ClassTemplate{
public:
    /* ------ Public Declarations ------  */
    struct Inner{
        char* msg { nullptr };
    };

    /* ------ Constructors ------  */
    /**
     * \~english
     * TODO_describe_ctor.
     *
     * \~chinese 这是一个类的模板
     * */
    ClassTemplate() {
    }

    /* ------ Destructor ------  */
    virtual ~ClassTemplate() {
    }

    /* --------- Public Methods --------  */

    /** TODO_describe_accessor.
     * @param param_a TODO_describe_input_param.
     * @return TODO_describe_return value. */
    int publicMethodA(int param_a);


    /* ------- Public Variables --------  */
    float f_val { 3.14f };  /* TODO_purpose_and_units. */

protected:
    /* ------ Protected Declarations ------  */
    /* --------- Protected Methods --------  */
    /* ------- Protected Variables --------  */

private:
    /* ------ Private Declarations ------  */
    /* --------- Private Methods --------  */
    /* ------- Private Variables --------  */
};

#endif //NAMESPACE_CLASSNAME_HPP