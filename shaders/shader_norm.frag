#version 430 core
out vec4 FragColor;

uniform sampler2D textureSampler;
uniform sampler2D normalMap;  

uniform vec3 lightDir;

in vec3 interpNormal;
in vec2 interpTexCoord;
in vec3 vertexWorldPosition;
in mat3 TBN;
in vec3 interpPos;

uniform vec3 cameraPos;
uniform samplerCube skybox;

void main()
{	
	vec2 modifiedTexCoord = vec2(interpTexCoord.x, 1.0 - interpTexCoord.y);
	vec3 color = texture2D(textureSampler, modifiedTexCoord).rgb;
	
	//vec3 normal = normalize(interpNormal);

	//normal mapping
	vec3 normal = texture(normalMap, interpTexCoord).rgb;
	normal = normalize(normal * 2.0 - 1.0);   
	//normal = normalize(TBN * normal); 

	float diffuse = max(dot(normal, -lightDir), 0.0);
	
	vec3 toEye = normalize(cameraPos - interpPos);
	float specular = pow(max(dot(toEye, reflect(lightDir, normal)), 0.0), 30.0);

	FragColor = vec4(color * diffuse, 1.0) + vec4(vec3(1.0) * specular, 0.0);
}
