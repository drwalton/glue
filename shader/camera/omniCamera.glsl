
vec2 dirToOmniImageLoc(in vec3 dir);

///\brief Get the location of a world space point in the omnidirecional image.
///\note If outside the image, returns vec2(-1,-1);
vec2 worldToOmniImageLoc(in mat4 cameraPose, in vec3 worldPos) {
	//Move point into camera-space.
	vec3 camSpacePos = vec3(cameraPose * vec4(worldPos, 1.f));
	camSpacePos = normalize(camSpacePos);
	
	//Check dot product (i.e. if point is directly behind camera).
	float dotProd = dot(camSpacePos, vec3(0.f, 0.f, 1.f));

	if(dotProd < minDotProduct) {
		return vec2(-1, -1);
	}
	
	//Move point into image space.
	float den = camSpacePos.z + length(camSpacePos) * e;
	vec2 imPos = vec2(
        fx * (camSpacePos.x / den) + cx,
        fy * (camSpacePos.y / den) + cy);
	
	if(length(imPos - c) > r) {
		return vec2(-1, -1);
	}
}

///\brief Get the location of a direction in the omnidirecional image.
///\note The direction should already be rotated appropriately.
///\note If outside the image, returns vec2(-1,-1);
vec2 dirToOmniImageLoc(in vec3 dir) {
	float dotProd = dot(dir, vec3(0.f, 0.f, 1.f));

	if(dotProd < minDotProduct) {
		return vec2(-1, -1);
	}
	
	//Move point into image space.
	float den = dir.z + length(dir) * e;
	vec2 imPos = vec2(
        fx * (dir.x / den) + cx,
        fy * (dir.y / den) + cy);
	
	if(length(imPos - c) > r) {
		return vec2(-1, -1);
	}
}
