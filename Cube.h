#pragma once
#include "src/model.h"
class Cube : public Model
{
	unsigned m_number_of_indices = 0;

public:
	// Constructor.
    Cube(ID3D11Device* dxdevice, ID3D11DeviceContext* dxdevice_context);

	// Render the model.
    virtual void Render() const;

	// Destructor.
	~Cube() {}
};