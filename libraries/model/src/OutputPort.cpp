////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Project:  Embedded Machine Learning Library (EMLL)
//  File:     OutputPort.cpp (model)
//  Authors:  Chuck Jacobs
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "OutputPort.h"

/// <summary> model namespace </summary>
namespace model
{
    OutputPortBase::OutputPortBase(const class Node* node, std::string name, PortType type, size_t size) : Port(node, name, type), _size(size), _isReferenced(false) 
    {}

    void OutputPortBase::AddProperties(utilities::Archiver& description) const
    {
        Port::AddProperties(description);
        description.SetType(*this);
        description["size"] << _size;

    }

    void OutputPortBase::SetObjectState(const utilities::Archiver& description, utilities::SerializationContext& context)
    {
        Port::SetObjectState(description, context);
        description["size"] >> _size;
    }    
}
