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

#include "Game_Scene.hpp"

#include <cstdlib>
#include <basics/Canvas>
#include <basics/Director>

using namespace basics;
using namespace std;

namespace project_template
{
    // ---------------------------------------------------------------------------------------------
    // ID y ruta de las texturas que se deben cargar para esta escena.

    //TODO: Implementar las texturas con su ID y ruta
    Game_Scene::Texture_Data Game_Scene::textures_data[] =
    {
        { ID(nombre_ID),  "game-scene/nombre_archivo.extension"},

        //...

    };

    // Para determinar el número de items en el array textures_data, se divide el tamaño en bytes
    // del array completo entre el tamaño en bytes de un item:

    unsigned Game_Scene::textures_count = sizeof(textures_data) / sizeof(Texture_Data);

    // ---------------------------------------------------------------------------------------------
    // Definiciones de los atributos estáticos de la clase:

    //TODO: Implementar los atributos estáticos de la clase
    //constexpr tipo_dato Game_Scene:: nombre_variable;


    // ---------------------------------------------------------------------------------------------

    Game_Scene::Game_Scene()
    {
        // Se establece la resolución virtual (independiente de la resolución virtual del dispositivo).
        // En este caso no se hace ajuste de aspect ratio, por lo que puede haber distorsión cuando
        // el aspect ratio real de la pantalla del dispositivo es distinto.

        canvas_width  = 1280;
        canvas_height =  720;

        aspect_ratio_adjusted = false;


        // Se inicia la semilla del generador de números aleatorios:
        srand (unsigned(time(nullptr)));

        // Se inicializan otros atributos:

        initialize ();
    }

    // ---------------------------------------------------------------------------------------------
    // Algunos atributos se inicializan en este método en lugar de hacerlo en el constructor porque
    // este método puede ser llamado más veces para restablecer el estado de la escena y el constructor
    // solo se invoca una vez.

    bool Game_Scene::initialize ()
    {
        state     = LOADING;
        suspended = true;
        gameplay  = UNINITIALIZED;

        return true;
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::suspend ()
    {
        suspended = true;               // Se marca que la escena ha pasado a primer plano
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::resume ()
    {
        suspended = false;              // Se marca que la escena ha pasado a segundo plano
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::handle (Event & event)
    {
        if (state == RUNNING)               // Se descartan los eventos cuando la escena está LOADING
        {
            if (gameplay == WAITING_TO_START)
            {
                start_playing ();           // Se empieza a jugar cuando el usuario toca la pantalla
                                            // por primera vez
            }
            else switch (event.id)
            {
                case ID(touch-started):     // El usuario toca la pantalla
                case ID(touch-moved):
                {
                    //Guardamos la posición tocada por el usuario
                    Point2f touch_location = { *event[ID(x)].as< var::Float > (),    //Coordenada X
                                               *event[ID(y)].as< var::Float > ()     //Coordenada Y
                                             };

                    break;
                }

                case ID(touch-ended):       // El usuario deja de tocar la pantalla
                {

                    break;
                }
            }
        }
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::update (float time)
    {
        if (!suspended) switch (state)
        {
            case LOADING: load_textures  ();     break;
            case RUNNING: run_simulation (time); break;
            case ERROR:   break;
        }
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::render (Context & context)
    {
        if (!suspended)
        {
            // El canvas se puede haber creado previamente, en cuyo caso solo hay que pedirlo:

            Canvas * canvas = context->get_renderer< Canvas > (ID(canvas));

            // Si no se ha creado previamente, hay que crearlo una vez:

            if (!canvas)
            {
                 canvas = Canvas::create (ID(canvas), context, {{ canvas_width, canvas_height }});
            }

            // Si el canvas se ha podido obtener o crear, se puede dibujar con él:

            if (canvas)
            {
                canvas->clear ();

                switch (state)
                {
                    case LOADING: render_loading   (*canvas); break;
                    case RUNNING: render_playfield (*canvas); break;
                    case ERROR:   break;
                }
            }
        }
    }

    // ---------------------------------------------------------------------------------------------
    // En este método solo se carga una textura por fotograma para poder pausar la carga si el
    // juego pasa a segundo plano inesperadamente. Otro aspecto interesante es que la carga no
    // comienza hasta que la escena se inicia para así tener la posibilidad de mostrar al usuario
    // que la carga está en curso en lugar de tener una pantalla en negro que no responde durante
    // un tiempo.

    void Game_Scene::load_textures ()
    {
        if (textures.size () < textures_count)          // Si quedan texturas por cargar...
        {
            // Las texturas se cargan y se suben al contexto gráfico, por lo que es necesario disponer
            // de uno:

            Graphics_Context::Accessor context = director.lock_graphics_context ();

            if (context)
            {
                // Se ajusta el aspect ratio si este no se ha ajustado
                if(!aspect_ratio_adjusted){

                    adjust_aspect_ratio(context);
                }

                // Se carga la siguiente textura (textures.size() indica cuántas llevamos cargadas):

                Texture_Data   & texture_data = textures_data[textures.size ()];
                Texture_Handle & texture      = textures[texture_data.id] = Texture_2D::create (texture_data.id, context, texture_data.path);

                // Se comprueba si la textura se ha podido cargar correctamente:

                if (texture) context->add (texture); else state = ERROR;

                // Cuando se han terminado de cargar todas las texturas se pueden crear los gameobjects que
                // las usarán e iniciar el juego:
            }
        }
        else
        if (timer.get_elapsed_seconds () > 1.f)         // Si las texturas se han cargado muy rápido
        {                                               // se espera un segundo desde el inicio de
            create_gameobjects();                          // la carga antes de pasar al juego para que
            restart_game   ();                          // el mensaje de carga no aparezca y desaparezca
                                                        // demasiado rápido.
            state = RUNNING;
        }
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::create_gameobjects()
    {

        //TODO: crear y configurar los gameobjects de la escena
        // Se crean y configuran los gameobjects:

        //GameObject_Handle  nombre_objeto(new GameObject (textures[ID(nombre_ID)].get() ));
        //...


        //Se establecen los anchor y position de los GameObject

        //nombre_objeto->set_anchor (...);
        //nombre_objeto->set_position ({coordenada_x, coordenada_y});


        //Se añaden a la lista de game objects
        //gameobjects.push_back (nombre_objeto);



        // Se guardan punteros a los gameobjects que se van a usar frecuentemente:

        // nombre_puntero = nombre_objeto.get();

    }

    // ---------------------------------------------------------------------------------------------
    // Cuando el juego se inicia por primera vez o cuando se reinicia porque un jugador pierde, se
    // llama a este método para restablecer los gameobjects:

    void Game_Scene::restart_game()
    {
        // TODO: resetear valores iniciales de los gameobjects que lo requieran
        //nombre_objeto->nombre_metodo(parametros);

        gameplay = WAITING_TO_START;
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::start_playing ()
    {
        //TODO: Implementar las cosas que se tengan que realizar al empezar a jugar

        gameplay = PLAYING;
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::run_simulation (float time)
    {
        // Se actualiza el estado de todos los gameobjects:

        for (auto & gameobject : gameobjects)
        {
            gameobject->update (time);
        }

        //TODO: implementación de la IA

        //TODO: implementación de posibles colisiones
    }


    // ---------------------------------------------------------------------------------------------

    void Game_Scene::render_loading (Canvas & canvas)
    {
        //TODO: tiene que haber alguna textura con ID loading para la carga
        Texture_2D * loading_texture = textures[ID(loading)].get ();

        if (loading_texture)
        {
            canvas.fill_rectangle
            (
                { canvas_width * .5f, canvas_height * .5f },
                { loading_texture->get_width (), loading_texture->get_height () },
                  loading_texture
            );
        }
    }

    // ---------------------------------------------------------------------------------------------
    // Simplemente se dibujan todos los gameobjects que conforman la escena.

    void Game_Scene::render_playfield (Canvas & canvas)
    {
        for (auto & gameobject : gameobjects)
        {
            gameobject->render (canvas);
        }
    }

    // ---------------------------------------------------------------------------------------------
    // Ajusta el aspect ratio

    void Game_Scene::adjust_aspect_ratio(Context & context)
    {

        float real_aspect_ratio = float( context->get_surface_width () ) / context->get_surface_height ();

        // TODO: Elegir qué disposición se desea mantener

        // Si se desea mantener el alto y ajustar el ancho (disposición horizontal)
        //canvas_width = unsigned( canvas_height * real_aspect_ratio );

        // Si se desea mantener el ancho y ajustar el alto (disposición vertical)
        //canvas_height = unsigned ( canvas_width * real_aspect_ratio);

        aspect_ratio_adjusted = true;
    }

}
