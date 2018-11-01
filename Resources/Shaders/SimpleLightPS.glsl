#version 330 core
out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

in vec3 Normal;
in vec3 FragPos;

void main()
{
	float ambientStrength = 0.1;
	vec3 ambient = ambientStrength * lightColor;

	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos); // Frag to light direction
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	float specularIntensity = 0.5;
	vec3 viewDir = normalize(viewPos - FragPos); // Frag to viewer
	vec3 reflectDir = reflect( -lightDir, norm );
	float spec = pow( max(dot(viewDir, reflectDir), 0.0 ), 32 );
	vec3 specular = specularIntensity * spec * lightColor;

	vec3 result = ( ambient + diffuse + specular ) * objectColor;
	FragColor = vec4(result, 1.0);
}