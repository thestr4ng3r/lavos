
// Peters C, Klein R. Moment shadow mapping

#define MSM_BIAS 0.00003
#define MSM_BLEEDING_REDUCTION 0.98

float MSMShadow(vec4 smpl, float frag_z)
{
	vec4 b = (1 - MSM_BIAS) * smpl + MSM_BIAS * vec4(0.5);
	float l32_d22 =				-b[0] * b[1] + b[2];
	float d22 =					-b[0] * b[0] + b[1];
	float sq_depth_variance =	-b[1] * b[1] + b[3];
	float d33_d22 = dot(vec2(sq_depth_variance, -l32_d22), vec2(d22, l32_d22));
	float inv_d22 = 1.0f / d22;
	float l32 = l32_d22 * inv_d22;
	vec3 z;
	z[0] = frag_z;
	vec3 c = vec3(1.0f, z[0], z[0] * z[0]);
	c[1] -= b.x;
	c[2] -= b.y + l32 * c[1];
	c[1] *= inv_d22;
	c[2] *= d22 / d33_d22;
	c[1] -= l32 * c[2];
	c[0] -= dot(c.yz, b.xy);
	float inv_c2 = 1.0f / c[2];
	float p = c[1] * inv_c2;
	float q = c[0] * inv_c2;
	float r = sqrt(p * p * 0.25f - q);
	z[1] = -p * 0.5f - r;
	z[2] = -p * 0.5f + r;
	vec4 sw = (z[2] < z[0])
		? vec4(z[1], z[0], 1.0f, 1.0f)
		: ((z[1] < z[0])
			? vec4(z[0], z[1], 0.0f, 1.0f)
			: vec4(0.0f, 0.0f, 0.0f, 0.0f));
	float quot = (sw[0] * z[2] - b[0] * (sw[0] + z[2]) + b[1]) / ((z[2] - sw[1]) * (z[0] - z[1]));
	float shadow = sw[2] + sw[3] * quot;
	shadow /= MSM_BLEEDING_REDUCTION;
	return 1.0 - clamp(shadow, 0.0, 1.0);
}