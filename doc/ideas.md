
ideas:

* strategy of placing new towers:
  * coverage:

    Each tower has coverage. it's a vector that determines how much tower do dmg across each route. also you can compute overall coverage by summing up coverage of all towers.

    When you want to place a new tower you have two goals: 1) maximize coverage of routes where creeps go through 2) balance coverage between all routes (2-nd one is probably not needed as we are able to place lots of towers, maybe we should take it into account on first steps of the game)

  * best (greedy):

    Each time choose tower that has: maximum coverage of broken routes
