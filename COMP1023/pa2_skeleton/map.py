from enum import IntEnum
from pathlib import Path
from tkinter import Canvas
from typing import Callable

import numpy as np

from utils import overlap
from assets import open_and_resize_photoimage, RESIZE_SCALE

TILE_SIZE = int(32 * RESIZE_SCALE)
ASSETS_PATH = Path("assets")


def create_map(map_str: str) -> "Map":
    """
    Read and parse a game map from a file.

    A map file is a text file where each non-blank character represents a tile in the map.

    A map file may contain the following characters:

    - "." represents an empty tile (Tile.EMPTY).
    - "#" represents a rock tile (Tile.ROCK).
    - "@" represents a bomb tile (Tile.BOMB).
    - "0" represents an empty tile and indicates the initial position of the tank for player 1.
    - "1" represents an empty tile and indicates the initial position of the tank for player 2.

    Each line in the file represents a row in the map and each non-blank character in the lines represents a tile.
    The number of non-space characters in the first line indicates the number of columns in the map.
    The number of lines in the file indicates the number of rows in the map.

    For example, the following is a valid map file:

    ```
    0 . .
    . # .
    . . 1
    ```

    The above map file represents a 3x3 map with the initial positions of tank 1 and tank 2, on the top-left and
    bottom-right corners, respectively. There is 1 rock tile in the center of the map with 8 empty tiles
    surrounding it.

    :param map_str: the file content as a string
    :return: the read and parsed game map
    """
    # a list of strings, each string is a line in the file
    lines = map_str.splitlines()

    # the number of lines in the file, i.e. the number of rows in the map
    rows = len(lines)

    # the number of non-space characters in the first line, i.e. the number of columns in the map
    cols = len(lines[0].split())

    # create a new Map object with the given number of columns and rows
    game_map = Map(cols, rows)

    # TODO: Task 1.1


    # TODO: Task 1.1 END
    return game_map


class Tile(IntEnum):
    """
    An enumeration of integer values to represent tiles in the map.
    You should use Tile.EMPTY, Tile.ROCK and Tile.BOMB to represent the corresponding tiles instead of 0, 1 and 2.
    """
    EMPTY = 0
    ROCK = 1
    BOMB = 2


class Map:
    """
    The Map class. Represents a map of the game.
    """
    def __init__(self, cols: int, rows: int):
        """
        Initializes the map.
        :param cols: the number of columns in the map
        :param rows: the number of rows in the map
        """
        self.cols = cols
        self.rows = rows
        self.width = cols * TILE_SIZE
        self.height = rows * TILE_SIZE

        self.prev_map = None
        self.map = np.full((rows, cols), Tile.EMPTY)
        self.canvas_floor_map = np.full((rows, cols), None)
        self.canvas_object_map = np.full((rows, cols), None)

        self.FLOOR_IMAGE = open_and_resize_photoimage(
            image_path=(ASSETS_PATH / "floor.png")
        )
        self.ROCK_IMAGE = open_and_resize_photoimage(
            image_path=(ASSETS_PATH / "rock.png")
        )
        self.BOMB_IMAGE = open_and_resize_photoimage(
            image_path=(ASSETS_PATH / "bomb.png")
        )

        self.tank_position_map = {}

    def map_diff(self):
        """
        Calculates the difference between the current map `self.map` and the previous map `self.prev_map`.

        If the previous map `self.prev_map` is None, it will be set to the current map `self.map` and the difference
        will be an array of True.

        After calculating the difference, the previous map `self.prev_map` will be updated to the current map
        `self.map`.

        YOU ARE NOT ALLOWED TO USE ANY LOOPS OR COMPREHENSION IN THIS TASK.

        :return: a boolean n-D array of the same shape as the map, where the element indicates whether the corresponding
        tile has changed.
        """
        # TODO: Task 1.2


        # TODO: Task 1.2 END

    def draw_on(self, canvas: Canvas):
        for y, x in np.argwhere(np.logical_not(self.canvas_floor_map)):
            self.canvas_floor_map[y][x] = canvas.create_image(
                x * TILE_SIZE, y * TILE_SIZE, image=self.FLOOR_IMAGE, anchor="nw"
            )

        diff = self.map_diff()

        for y, x in np.argwhere(diff):
            tile = self.map[y][x]
            canvas.delete(self.canvas_object_map[y][x])  # type: ignore
            if tile == Tile.ROCK:
                self.canvas_object_map[y][x] = canvas.create_image(
                    x * TILE_SIZE, y * TILE_SIZE, image=self.ROCK_IMAGE, anchor="nw"
                )
            if tile == Tile.BOMB:
                self.canvas_object_map[y][x] = canvas.create_image(
                    x * TILE_SIZE, y * TILE_SIZE, image=self.BOMB_IMAGE, anchor="nw"
                )

    def collides(
        self, x: float, y: float, width: float = 0, height: float = 0
    ) -> dict[tuple[int, int], Tile] | None:
        """
        Check if an object (not the 'object' in OOP >.<) whose hit-box centered at (x, y) with the size (width, height)
        collides with the map.

        An object collides with the map if:

        1. the object's hit-box, which means the rectangle it occupies in this assignment, overlaps with a non-empty tile, OR
        2. the object's hit-box, or partial hit-box is outside or overlaps with the map boundary.

        In the 1st case, the function returns a dictionary indicating the tile(s) that the object collides with; the
        key is the (x, y) coordinate of the top left corner of the tile(s), and the value is the type of the tile(s).
        Example: {(0, 0): Tile.BOMB}

        In the 2nd case, the function returns None. The 2nd case takes precedence over the 1st case.

        If neither of the two cases happens, the function returns an empty dictionary.

        Overlaps are inclusive, that is, if the hit-box of the object touches the map boundary or a tile,
        it is considered as overlapping.

        :param x: the x-coordinate of the center of the object's hit-box
        :param y: the y-coordinate of the center of the object's hit-box
        :param width: the width of the object's hit-box
        :param height: the height of the object's hit-box
        :return: a dictionary or None
        """
        # TODO: Task 2.1


        # TODO: Task 2.1 END

    def collides_with_tank(
        self, x: float, y: float, width: float = 0, height: float = 0
    ) -> int | None:
        """
        Check if an object whose hit-box centered at (x, y) with the size (width, height) collides with a tank.

        an object collides with the tank if its hit-box overlaps with the tank's hit-box.

        Overlaps are inclusive, that is, if the hit-box of the object touches the map boundary or a tile,
        it is considered as overlapping.

        The size of the tank's hit-box is 1x1.

        You may assume that the object will not collide with multiple tanks at the same time.

        :param x: the x-coordinate of the center of the object's hit-box
        :param y: the y-coordinate of the center of the object's hit-box
        :param width: the width of the object's hit-box
        :param height: the height of the object's hit-box
        :return: the tank id that the object collides with, or None if the object does not collide with any tank
        """
        # TODO: Task 2.2


        # TODO: Task 2.2 END

    def nearest_position(
        self,
        x: float,
        y: float,
        new_x: float,
        new_y: float,
        width: float = 0,
        height: float = 0,
    ) -> tuple[float, float]:
        """
        Find the nearest position from (x, y) to (new_x, new_y) that does not collide with the map.
        """
        if self.collides(new_x, new_y, width, height) == {}:
            return new_x, new_y
        else:
            if new_x != x:
                new_x, _ = self.nearest_position(
                    x, y, new_x + np.sign(x - new_x) / 32, new_y, width, height
                )
            if new_y != y:
                _, new_y = self.nearest_position(
                    x, y, new_x, new_y + np.sign(y - new_y) / 32, width, height
                )
            return new_x, new_y

    def trigger_bomb(
        self, canvas: Canvas, col: int, row: int, explode: Callable[[int, int], None]
    ) -> None:
        """
        Trigger a bomb at the given position (col, row) and explode the surrounding tiles.

        The bomb will explode the surrounding tiles in a 3x3 square centered at the bomb, including the bomb itself.
        That is, all the tiles in the 3x3 square will be set to Tile.EMPTY.

        If another bomb is exploded by the current bomb, that is, if there are other bombs in the 3x3 square,
        the bomb will trigger these bombs, causing a chain reaction. You may call this function recursively to
        trigger the explosion of another bomb. Note that in calling this function recursively, you should pass the same
        `canvas` object and `explode` function.
        Do not use `canvas.after()` or some other similar functions to schedule the triggering.
        Otherwise, you may get trouble during grading.

        You can assume that there is indeed a bomb at the [row][column] position specified by the parameters.
        Moreover, you should NOT call `trigger_bomb()` on a position if there is not a bomb there.

        After modifying the map, for example, setting a tile to `Tile.EMPTY`, you should call
        `self.draw_on(canvas)` to update the canvas.

        :param canvas: the canvas to draw on
        :param col: the column of the bomb
        :param row: the row of the bomb
        :param explode: a function that triggers the explosion animation at the given position (col, row)
        """

        # Trigger the explosion animation at the bomb position.
        explode(col, row)

        # Example: the tile at (col, row) is a bomb, so set it to Tile.EMPTY.
        # Important: After modifying the map, call `self.draw_on(canvas)` to update the canvas.
        self.map[row][col] = Tile.EMPTY
        self.draw_on(canvas)

        # You may start to implement the remaining part of the function here.

        # TODO 3


        # TODO 3 END
