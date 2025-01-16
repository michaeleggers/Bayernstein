import numpy as np
from data_structures.triangle import Triangle
from data_structures.bounding_box import BoundingBox
from data_structures.vector3f import Vector3f
from typing import List, Optional
import util.geometry as geometry

class BVHNode:
    def __init__(self, bounding_box: BoundingBox, triangles: List["Triangle"], left: Optional["BVHNode"] = None, right: Optional["BVHNode"] = None):
        self.bounding_box = bounding_box
        self.triangles = triangles
        self.left = left
        self.right = right

    def is_leaf(self):
        return self.left is None and self.right is None


def build_bvh(triangles: List["Triangle"], max_triangles_per_leaf=4) -> BVHNode:
    if len(triangles) <= max_triangles_per_leaf:
        # Leaf node
        return BVHNode(geometry.bbox_from_triangles(triangles), triangles)

    # Split triangles by their centroid along the longest axis
    centroids = [np.mean([v.to_array() for v in t.vertices], axis=0) for t in triangles]
    centroids = np.array(centroids)
    axis = np.argmax(np.ptp(centroids, axis=0))  # Choose axis with the largest spread
    sorted_triangles = [t for _, t in sorted(zip(centroids[:, axis], triangles), key=lambda x: x[0])]

    mid = len(triangles) // 2
    left_triangles = sorted_triangles[:mid]
    right_triangles = sorted_triangles[mid:]

    # Recursively build child nodes
    left_child = build_bvh(left_triangles, max_triangles_per_leaf)
    right_child = build_bvh(right_triangles, max_triangles_per_leaf)
    bounding_box = geometry.bbox_from_triangles(triangles)

    return BVHNode(bounding_box, [], left_child, right_child)

def intersect_bvh(ray_origin: Vector3f, ray_direction: Vector3f, node: BVHNode) -> Optional[float]:
    if not node.bounding_box.ray_intersects(ray_origin, ray_direction):
        return None

    if node.is_leaf():
        # Test all triangles in the leaf
        closest_t = float("inf")
        for triangle in node.triangles:
            t = triangle.intersect_ray(ray_origin, ray_direction)
            if t is not None and t < closest_t:
                closest_t = t
        return closest_t if closest_t < float("inf") else None

    # Recursively test child nodes
    left_t = intersect_bvh(ray_origin, ray_direction, node.left) if node.left else None
    right_t = intersect_bvh(ray_origin, ray_direction, node.right) if node.right else None

    if left_t is not None and right_t is not None:
        return min(left_t, right_t)
    return left_t or right_t