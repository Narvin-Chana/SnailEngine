# Utilisation

## Déplacement du véhicule

Le fonctionnement actuel utilise la voiture EngineVehicle de PhysX.

Le déplacement du véhicule côté joueur s'effectue avec les touches suivante sur clavier ou la manette :

### Clavier :

`W` - Avancer

`S` - Reculer/Freiner

`A` - Tourner vers la gauche

`D` - Tourner vers la droite

`R` - Redresser la voiture

`E` - Utiliser le boost

### Manette:

`RT` - Avancer

`LT` - Reculer/Freiner

`LJ` - Tourner (droite et gauche)

`A` - Redresser la voiture

`B` - Utiliser le boost

## Déplacement de la caméra

La camera est attachée au véhicule.
Il existe deux modes : première personne et troisième personne.

Clavier:

`F` - Changer de vue FPS/TPS

Manette:

`Y` - Changer de vue FPS/TPS

## Collisions

Les collisions sont gérées par PhysX. On utilise 3 type de collisions:
- Trigger pour ce qui est boost, clé, adjustement du volumetric lighting dans le tunnel, et les checkpoints.
- Raycast pour les roues de la voiture sur le sol (éviter que la voiture tombe à travers).
- Simulation pour toutes les collisions voulues réalistes (objet sur la route, collision voiture-mur, etc...).

## Niveau de détail

Il existe 2 méthodes pour cette partie. 
La première est le découpage de notre terrain en chunks, on affiche uniquement les parties du terrain qui sont dans le frustum de la caméra.
La seconde est le frustum culling général, on affiche uniquement les objets qui sont dans le champ de vision de la caméra.

## Affichage du circuit et du terrain

Le terrain est un fichier `.obj` qui a été généré à partir d'une heightmap sur Blender. Il se trouve dans le dossier SnailEngine/Resources/Heightmaps.
Le circuit est réalisé par du blending de textures : une texture carrée noir et blanc permet cela. Elle se trouve dans le dossier SnailEngine/Resources.

## Affichage des objets

Le moteur affiche les objets au format `.obj`.
Les objets de la scene sont enregistrés et modifiables grâce à un fichier de séralisation (json) de la scene. 
On peut les retrouver dans SnailEngine/Resources/Scenes.

## Affichage des lumières

Nous utilisons Blinn-Phong pour l'éclairage (Directional Lights, Point Light et Cone Lights). 
Notre scène principale comporte un directional light et plusieurs point lights.
Les point lights sont sous la forme de lucioles dans notre tunnel.

## Menu principal

Le menu principal est assez simple, il permet de lancer une partie et de quitter le jeu.

On peut y voir la voiture qui tourne sur elle même.

## Menu pause

Pour afficher le menu pause, appuyez sur la touche `ESC`. On trouve alors un bouton pour redémarrer la partie, pour revenir au jeu (avec la touche Echap aussi), pour revenir au menu principal et pour quitter le jeu.

## Réaliser une partie

Pour finir une partie il faut réaliser trois tours du circuit en passant par les différents checkpoints (invisibles mais positionnés de manière que le joueur passe dedans). A chaque tour, un son est émis au passage de la ligne de départ pour valider la réussite du tour.

## Identification des risques restants (Coupe / Rebudgeté, par rapport au volet #1)

Les changements principaux par rapport au volet 1 sont les suivant:

- Pas de fourmis qui bloquent le passage.

- Le boost est devenu une boisson énergisante et plus une allumette.

- Pas de rétrécissement mais une clé qui déverrouille un accès du circuit.

- Nous sommes passés d'un tuyau d'arrosage à une canalisation avec des trous.

- Le nombre de tours est fixé, le joueur ne le choisit pas.

- Il n'y a pas de tableau des scores global, seulement son résultat personnel a chaque course.

- On ne peut pas modifier le skin de la voiture comme initialement prévu.

## Points de complexité mis en place

Techs:
+1 - Normal mapping

+1 - Texture Masking

+1 - Transparents (UI)

+2 - Ombres (Shadow Maps / CSM)

+2 - Projective decals

+3? - si Ombres volumétriques == Lumière Volumétrique

+? - GPU Instancing for Procedural Grass

Post Effects:

+1 - Screen Shake

+1 - Chromatic Aberration

+1 - Vignette

+2 - SSAO

+3 - FXAA

------------------------

total = 14 (ou 17 avec le volumétrique)

## Librairies utilisées
- DX11
- DirectXTK Audio
- XInput
- DXMath et SimpleMath
- ImGui
- PhysX
- nlohmann::json et rapidjson
- stb_image
- renderdoc
- rapidobj

## Fichiers importants
* Engine.h: contient la classe Engine qui est le moteur du jeu, c'est lui qui gère la boucle de jeu et qui appelle les fonctions de rendu et de mise à jour.

* WindowsEngine.h et WindowsEngine.cpp : fait le lien avec la fenêtre windows

* Scene.h et Scene.cpp : contient la classe Scene qui gère la scène, c'est elle qui charge les objets, les lumières, les caméras, etc...

* RendererModule.h et RendererModule.cpp : contient l'ensemble de notre pipeline de rendu

* CameraManager.h et CameraManager.cpp : contient la classe CameraManager qui gère les caméras, c'est elle qui gère les changements de caméra

* Mesh.h et Mesh.cpp : contient la classe Mesh qui gère les objets, c'est elle qui charge les objets et qui les affiche (notamment pour le terrain)

* PhysxVehicle.h et PhysxVehicle.cpp : contient la classe qui gère l'ensemble des paramètres physiques de la voiture.

## NB
Il se peut que la voiture n'avance pas en release (une fois sur trois environ). Il suffit de faire Échap->Restart pour recharger la scène.
Nous avons cherché longtemps pour trouver pourquoi mais nous n'avons pas réussi :(