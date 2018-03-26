#version 450 core

subroutine vec4 sub_mySubroutine(vec4 param);

subroutine (sub_mySubroutine)
vec4 myFunction1(vec4 param)
{
	return param * vec4(1.0, 0.25, 0.25, 1.0);
}

subroutine (sub_mySubroutine)
vec4 myFunction2(vec4 param)
{
	return param * vec4(0.25, 0.25, 1.0, 1.0);
}

subroutine uniform sub_mySubroutine mySubroutineUniform;

out vec4 color;

void main(void)
{
	color = mySubroutineUniform(vec4(1.0));
}