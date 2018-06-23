#version 430 core

uniform sampler2D textureSampler;
uniform sampler2D shadowMap;
uniform vec3 lightDir;
uniform vec3 cameraPos;

in vec3 interpNormal;
in vec3 interpPos;
in vec2 interpTexCoord;
in vec4 lightPos;


float ShadowCalculation(vec4 lightPos)
{
    vec3 projCoords = lightPos.xyz / lightPos.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    float shadow = currentDepth - 0.05 > closestDepth  ? 1.0 : 0.0;
    return shadow;
}  

void main()
{
	vec2 modifiedTexCoord = vec2(interpTexCoord.x, 1.0 - interpTexCoord.y);
	vec3 color = texture2D(textureSampler, modifiedTexCoord).rgb;
	
	vec3 normal = normalize(interpNormal);
	float diffuse = max(dot(normal, -lightDir), 0.0);
	
	vec3 toEye = normalize(cameraPos - interpPos);
	float specular = pow(max(dot(toEye, reflect(lightDir, normal)), 0.0), 30.0);

	float shadow = ShadowCalculation(lightPos);        

	gl_FragColor = vec4((1.0 - shadow) * color * diffuse, 1.0) + vec4(vec3(1.0) * specular, 0.0);
}


