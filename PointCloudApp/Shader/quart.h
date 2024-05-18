struct Quaternion {
  float x;
  float y;
  float z;
  float w;
};

Quaternion identity() {
  return Quaternion(0.0, 0.0, 0.0, 1.0);
}

Quaternion axisAngle(vec3 axis, float radian) {
  vec3 naxis = normalize(axis);
  float h = 0.5 * radian;
  float s = sin(h);
  return Quaternion(naxis.x * s, naxis.y * s, naxis.z * s, cos(h));
}

Quaternion conjugate(Quaternion q) {
  return Quaternion(-q.x, -q.y, -q.z, q.w);
}

Quaternion add(Quaternion q1, Quaternion q2) {
  return Quaternion(
    q1.x + q2.x,
    q1.y + q2.y,
    q1.z + q2.z,
    q1.w + q2.w
  );
}

Quaternion sub(Quaternion q1, Quaternion q2) {
  return Quaternion(
    q1.x - q2.x,
    q1.y - q2.y,
    q1.z - q2.z,
    q1.w - q2.w
  );
}

Quaternion mul(Quaternion q, float f) {
  return Quaternion(f * q.x, f * q.y, f * q.z, f * q.w);
}

Quaternion mul(Quaternion q1, Quaternion q2) {
  return Quaternion(
    q2.w * q1.x - q2.z * q1.y + q2.y * q1.z + q2.x * q1.w,
    q2.z * q1.x + q2.w * q1.y - q2.x * q1.z + q2.y * q1.w,
    -q2.y * q1.x + q2.x * q1.y + q2.w * q1.z + q2.z * q1.w,
    -q2.x * q1.x - q2.y * q1.y - q2.z * q1.z + q2.w * q1.w
  );
}

float qdot(Quaternion q1, Quaternion q2) {
  return q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
}

float squareNorm(Quaternion q) {
  return q.x * q.x + q.y * q.y + q.z * q.z + q.w + q.w;
}

float norm(Quaternion q) {
  return sqrt(squareNorm(q));
}

Quaternion qinverse(Quaternion q) {
  Quaternion c = conjugate(q);
  float s = norm(q);
  return mul(c, 1.0 / s);
}

vec3 rotate(vec3 v, Quaternion q) {
  // norm of q must be 1.
  Quaternion vq = Quaternion(v.x, v.y, v.z, 0.0);
  Quaternion cq = conjugate(q);
  Quaternion mq = mul(mul(cq, vq), q);
  return vec3(mq.x, mq.y, mq.z);
}

Quaternion Vec4ToQuart(vec4 q)
{
	return Quaternion(q.x,q.y,q.z,q.w);
}

vec4 QuartToVec4(Quaternion q)
{
	return vec4(q.x,q.y,q.z,q.w);
}


mat4 QuartToMat4( vec4 q ){
    return mat4(
        1.0 - 2.0 * q.y * q.y - 2.0 * q.z * q.z, 2.0 * q.x * q.y + 2.0 * q.w * q.z, 2.0 * q.x * q.z - 2.0 * q.w * q.y, 0.0,
        2.0 * q.x * q.y - 2.0 * q.w * q.z, 1.0 - 2.0 * q.x * q.x - 2.0 * q.z * q.z, 2.0 * q.y * q.z + 2.0 * q.w * q.x, 0.0,
        2.0 * q.x * q.z + 2.0 * q.w * q.y, 2.0 * q.y * q.z - 2.0 * q.w * q.x, 1.0 - 2.0 * q.x * q.x - 2.0 * q.y * q.y, 0.0,
        0.0, 0.0, 0.0, 1.0);
}

mat4 TranslateToMat4(vec4 d)
{
	return mat4(
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		d.x,d.y,d.z,1);
}

mat4 ScaleToMat4(vec4 d)
{
	return mat4(
		d.x,0,0,0,
		0,d.y,0,0,
		0,0,d.z,0,
		0,0,0,1);
}

Quaternion slerp(Quaternion q1, Quaternion q2, float t) {
  float cosine = qdot(q1, q2);
  if (cosine < 0.0) {
    cosine = qdot(q1, mul(q2, -1.0));
  }
  float r = acos(qdot(q1, q2));
  float is = 1.0 / sin(r);
  return add(
    mul(q1, sin((1.0 - t) * r) * is),
    mul(q2, sin(t * r) * is)
  );
}


vec4 slerp(vec4 q1, vec4 q2, float t) {
	return QuartToVec4(slerp(Vec4ToQuart(q1),Vec4ToQuart(q2),t));
}