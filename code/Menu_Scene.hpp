/*
 * MENU SCENE
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


#ifndef MENU_SCENE_HEADER
#define MENU_SCENE_HEADER

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

        class Menu_Scene : public basics::Scene
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
                READY,
                ERROR
            };

            /**
            * Array de estructuras con la información de las texturas (Id y ruta) que hay que cargar.
            */
            static struct   Texture_Data { Id id; const char * path; } textures_data[];

            /**
             * Número de items que hay en el array textures_data.
             */
            static unsigned textures_count;


        private:

            State          state;                               ///< Estado de la escena.
            bool           suspended;                           ///< true cuando la escena está en segundo plano y viceversa.
            bool           showing_instructions;

            unsigned       canvas_width;                        ///< Ancho de la resolución virtual usada para dibujar.
            unsigned       canvas_height;                       ///< Alto  de la resolución virtual usada para dibujar.
            bool           aspect_ratio_adjusted;               ///< False hasta que se ajuste el aspect ratio de la resolución.
            float          real_aspect_ratio;

            Texture_Map        textures;                        ///< Mapa  en el que se guardan shared_ptr a las texturas cargadas.
            GameObject_List    buttons;                         ///< Lista en la que se guardan shared_ptr a los gameobject creados.

            Timer          timer;                               ///< Cronómetro usado para medir intervalos de tiempo

            GameObject *         play_button_pointer;                          ///< Puntero al botón de jugar
            GameObject * instructions_button_pointer;                          ///< Puntero al botón de instrucciones
            GameObject * instructions_text_pointer;                            ///< Puntero al texto de instrucciones
            GameObject * logo_pointer;                                         ///< Puntero al logo


        public:

            Menu_Scene();

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
            void suspend () override
            {
                suspended = true;
            }

            /**
             * Este método lo invoca Director automáticamente cuando el juego pasa a primer plano.
             */
            void resume () override
            {
                suspended = false;
            }

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
             * Ajusta el aspect ratio al dispositivo
             */
            void adjust_aspect_ratio(Context & context);

            /**
             * En este método se crean los gameobjects cuando termina la carga de texturas.
             */
            void create_gameobjects();

            /**
            * Actualiza el estado del juego cuando el estado de la escena es READY.
            */
            void run_simulation (float time);

            /**
             * Dibuja la escena del menú cuando el estado de la escena es READY.
             * @param canvas Referencia al Canvas con el que dibujar.
             */
            void render_playfield (Canvas & canvas);

            /**
             * Empieza la partida
             */
            void play ();

            /**
             * Muestra las instrucciones
             */
            void show_instructions (bool _showing);

        };

    }

#endif
