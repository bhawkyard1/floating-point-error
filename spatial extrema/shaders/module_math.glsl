float distanceSquared(vec3 _a, vec3 _b)
{
  return (_b.x - _a.x) * (_b.x - _a.x) +
      (_b.y - _a.y) * (_b.y - _a.y) +
      (_b.z - _a.z) * (_b.z - _a.z);
}
