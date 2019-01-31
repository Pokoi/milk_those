/*
 * GAME SCENE
 * Copyright © 2018+ Ángel Rodríguez Ballesteros
 *
 * Distributed under the Boost Software License, version  1.0
 * See documents/LICENSE.TXT or www.boost.org/LICENSE_1_0.txt
 *
 * angel.rodriguez@esne.edu
 */

/*
 * MODIFIED BY
 *
 * Jesus 'Pokoi' Villar
 * © pokoidev 2019 (pokoidev.com)
 *
 * Creative Commons License:
 * Attribution 4.0 International (CC BY 4.0)
 *
 */

#ifndef GAME_SCENE_HEADER
#define GAME_SCENE_HEADER

    #include <map>
    #include <list>
    #include <memory>

    #include <basics/Canvas>
    #include <basics/Id>
    #include <basics/Scene>
    #include <basics/Texture_2D>
    #include <basics/Timer>

    #include "GameObject.hpp"

    namespace project_template
    {

        using basics::Id;
        using basics::Timer;
        using basics::Canvas;
        using basics::Texture_2D;

        class Game_Scene : public basics::Scene
        {

            // Estos typedefs pueden ayudar a hacer el código más compacto y claro:

            typedef std::shared_ptr < GameObject >         GameObject_Handle;
            typedef std::vector< GameObject_Handle >       GameObject_List;
            typedef std::shared_ptr< Texture_2D  >         Texture_Handle;
            typedef std::map< Id, Texture_Handle >         Texture_Map;
            typedef basics::Graphics_Context::Accessor     Context;

            /**
             * Representa el estado de la escena en su conjunto.
             */
            enum State
            {
                LOADING,
                RUNNING,
                ERROR
            };

            /**
             * Representa el estado del juego cuando el estado de la escena es RUNNING.
             */
            enum Gameplay_State
            {
                UNINITIALIZED,
                WAITING_TO_START,
                PLAYING,
                ENDING,
            };

        private:

            /**
             * Array de estructuras con la información de las texturas (Id y ruta) que hay que cargar.
             */
            static struct   Texture_Data { Id id; const char * path; } textures_data[];

            /**
             * Número de items que hay en el array textures_data.
             */
            static unsigned textures_count;

            /**
             * Cantidad de litros obtenidos por el jugador
             */
            static float liters;

        private:

            static constexpr size_t  bullet_amount =    50;             ///< Cantidad de proyectiles de leche
            static constexpr float   bullet_speed  =  -200;              ///< Velocidad de los proyectiles
            static constexpr float   milk_for_shot = 0.10f;             ///< Cantidad de leche que proporciona un proyectil
            static constexpr int     max_time      =    60;                  ///< Cantidad máxima de segundos de partida


        private:

            State          state;                               ///< Estado de la escena.
            Gameplay_State gameplay;                            ///< Estado del juego cuando la escena está RUNNING.
            bool           suspended;                           ///< true cuando la escena está en segundo plano y viceversa.
            bool           game_paused;

            unsigned       canvas_width;                        ///< Ancho de la resolución virtual usada para dibujar.
            unsigned       canvas_height;                       ///< Alto  de la resolución virtual usada para dibujar.
            bool           aspect_ratio_adjusted;               ///< False hasta que se ajuste el aspect ratio de la resolución.
            float          real_aspect_ratio;

            Texture_Map        textures;                        ///< Mapa  en el que se guardan shared_ptr a las texturas cargadas.
            GameObject_List    gameobjects;                     ///< Lista en la que se guardan shared_ptr a los gameobject creados.
            GameObject_List    bullets;                         ///< Lista en la que se guardan shared_ptr a los proyectiles creados.

            Timer          timer;                               ///< Cronómetro usado para medir intervalos de tiempo
            Point2f        first_spawn_position;                ///< Posición del punto de spwan de la primera ubre
            Point2f        second_spawn_position;               ///< Posición del punto de spwan de la segunda ubre
            unsigned       last_udder_clicked;                  ///< La última ubre pulsada por el jugador

            GameObject * first_udder_pointer ;                          ///< Puntero a la primera ubre
            GameObject * second_udder_pointer;                          ///< Puntero a la segunda ubre
            GameObject * bucket_pointer;                                ///< Puntero al cubo
            GameObject * pausa_button_pointer;                          ///< Puntero al botón de pausa
            GameObject * pausa_text_pointer;                            ///< Puntero al texto de pausa



        public:

            /**
             * Solo inicializa los atributos que deben estar inicializados la primera vez, cuando se
             * crea la escena desde cero.
             */
            Game_Scene();

            /**
             * Este método lo llama Director para conocer la resolución virtual con la que está
             * trabajando la escena.
             * @return Tamaño en coordenadas virtuales que está usando la escena.
             */
            basics::Size2u get_view_size () override
            {
                return { canvas_width, canvas_height };
            }

            /**
             * Aquí se inicializan los atributos que deben restablecerse cada vez que se inicia la escena.
             * @return
             */
            bool initialize () override;

            /**
             * Este método lo invoca Director automáticamente cuando el juego pasa a segundo plano.
             */
            void suspend () override;

            /**
             * Este método lo invoca Director automáticamente cuando el juego pasa a primer plano.
             */
            void resume () override;

            /**
             * Este método se invoca automáticamente una vez por fotograma cuando se acumulan
             * eventos dirigidos a la escena.
             */
            void handle (basics::Event & event) override;

            /**
             * Este método se invoca automáticamente una vez por fotograma para que la escena
             * actualize su estado.
             */
            void update (float time) override;

            /**
             * Este método se invoca automáticamente una vez por fotograma para que la escena
             * dibuje su contenido.
             */
            void render (Context & context) override;

        private:

            /**
             * En este método se cargan las texturas (una cada fotograma para facilitar que la
             * propia carga se pueda pausar cuando la aplicación pasa a segundo plano).
             */
            void load_textures ();

            /**
             * En este método se crean los gameobjects cuando termina la carga de texturas.
             */
            void create_gameobjects();

            /**
             * Se llama cada vez que se debe reiniciar el juego. En concreto la primera vez y cada
             * vez que un jugador pierde.
             */
            void restart_game ();

            /**
             * Cuando se ha reiniciado el juego
             */
            void start_playing ();

            /**
             * Actualiza el estado del juego cuando el estado de la escena es RUNNING.
             */
            void run_simulation (float time);


            /**
             * Dibuja la escena de juego cuando el estado de la escena es RUNNING.
             * @param canvas Referencia al Canvas con el que dibujar.
             */
            void render_playfield (Canvas & canvas);

            /**
             * Ajusta el aspect ratio
             */
            void adjust_aspect_ratio(Context & context);


            /**
             * Spawnea un proyectil en la posición dada y se le aplica una velocidad
             */
            void spawn_bullet (const Point2f & point);

            /**
             * Método que pausa el juego
             */
            void pause_the_game (bool paused);



        };

    }

#endif
