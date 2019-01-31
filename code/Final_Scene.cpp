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



#include "Game_Scene.hpp"
#include "Final_Scene.hpp"

#include <basics/Canvas>
#include <basics/Director>
#include <basics/Transformation>

using namespace basics;
using namespace std;

namespace project_template
{

    // ---------------------------------------------------------------------------------------------
    // ID y ruta de las texturas que se deben cargar para esta escena.

    //TODO: Implementar las texturas con su ID y ruta
    Final_Scene::Texture_Data Final_Scene::textures_data[] =
            {
                    //{ ID(nombre_ID),  "game-scene/nombre_archivo.extension"},
                    { ID(play_button_id),          "menu/jugar.png"},
                    { ID(salir_button_id),         "final/salir.png"},
                    { ID(zero_points_id),          "final/0-puntos.png"},
                    { ID(one_points_id),           "final/1-punto.png"},
                    { ID(two_points_id),           "final/2-puntos.png"},
                    { ID(three_points_id),         "final/3-puntos.png"},


                    //...

            };

    // Para determinar el número de items en el array textures_data, se divide el tamaño en bytes
    // del array completo entre el tamaño en bytes de un item:

    unsigned Final_Scene::textures_count = sizeof(textures_data) / sizeof(Texture_Data);

    Final_Scene::Final_Scene()
    {
        state         = LOADING;
        suspended     = true;
        canvas_width  = 720;
        canvas_height = 1280;

    }

    Final_Scene::Final_Scene(float _litters)
    {
        state         = LOADING;
        suspended     = true;
        canvas_width  = 720;
        canvas_height = 1280;
        aspect_ratio_adjusted = false;

        if      (_litters < one_point_achivement)                                       points = 0;
        else if (_litters >= one_point_achivement && _litters < two_point_achivement)   points = 1;
        else if (_litters >= two_point_achivement && _litters < three_point_achivement) points = 2;
        else if (_litters >= three_point_achivement)                                    points = 3;
    }

    // ---------------------------------------------------------------------------------------------

    bool Final_Scene::initialize ()
    {
        state     = LOADING;
        suspended = true;

        return true;
    }

    // ---------------------------------------------------------------------------------------------

    void Final_Scene::handle (basics::Event & event)
    {
        if (state == READY)                     // Se descartan los eventos cuando la escena está LOADING
        {
            switch (event.id)
            {
                case ID(touch-started):         // El usuario toca la pantalla
                {
                    Point2f touch_location = { *event[ID(x)].as< var::Float > (), *event[ID(y)].as< var::Float > () };

                    if      (play_button_pointer         -> contains(touch_location)) play();
                    else if (salir_button_pointer -> contains(touch_location)) director.stop();

                    break;

                }
                case ID(touch-moved):
                {
                    break;
                }

                case ID(touch-ended):
                {

                    break;
                }
            }
        }
    }

    // ---------------------------------------------------------------------------------------------

    void Final_Scene::update (float time)
    {

        if (!suspended) switch (state)
            {
                case LOADING: load_textures  ();     break;
                case READY: run_simulation (time); break;
                case ERROR:   break;
            }

    }

    // ---------------------------------------------------------------------------------------------

    void Final_Scene::render (Graphics_Context::Accessor & context)
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
                    case LOADING: break;
                    case READY:   render_playfield (*canvas); break;
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

    void Final_Scene::load_textures ()
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
        //if (timer.get_elapsed_seconds () > 1.f)         // Si las texturas se han cargado muy rápido
        {                                               // se espera un segundo desde el inicio de
            create_gameobjects();                       // la carga antes de pasar al juego para que
            state = READY;                              // el mensaje de carga no aparezca y desaparezca
                                                        // demasiado rápido.

        }
    }


    // ---------------------------------------------------------------------------------------------
    // Ajusta el aspect ratio

    void Final_Scene::adjust_aspect_ratio(Context & context)
    {

        real_aspect_ratio = float( context->get_surface_width () ) / context->get_surface_height ();

        // TODO: Elegir qué disposición se desea mantener

        // Si se desea mantener el alto y ajustar el ancho (disposición horizontal)
        //canvas_width = unsigned( canvas_height * real_aspect_ratio );

        // Si se desea mantener el ancho y ajustar el alto (disposición vertical)
        canvas_height = unsigned ( canvas_width * real_aspect_ratio);

        aspect_ratio_adjusted = true;
    }


    // ---------------------------------------------------------------------------------------------
    // Crea los objetos de la escena

    void Final_Scene::create_gameobjects()
    {

        //TODO: crear y configurar los gameobjects de la escena
        // 1) Se crean y configuran los gameobjects:

        //GameObject_Handle  nombre_objeto(new GameObject (textures[ID(nombre_ID)].get() ));
        //...

        // 2) Se establecen los anchor y position de los GameObject
        // Si no se especifica se colocan por defecto en la posición (0,0) y el anchor en el centro
        //nombre_objeto->set_anchor (...);
        //nombre_objeto->set_position ({coordenada_x, coordenada_y});

        // 3) Se añaden a la lista de game objects
        //gameobjects.push_back (nombre_objeto);


        // Se crean los objetos no dinámicos de la escena
        GameObject_Handle play_button_object         (new GameObject (textures[ID(play_button_id)]. get(), real_aspect_ratio));
        GameObject_Handle salir_button_object        (new GameObject (textures[ID(salir_button_id)]. get(), real_aspect_ratio));
        GameObject_Handle zero_points_object         (new GameObject (textures[ID(zero_points_id)].get(), real_aspect_ratio));
        GameObject_Handle one_points_object          (new GameObject (textures[ID(one_points_id)].get(), real_aspect_ratio));
        GameObject_Handle two_points_object          (new GameObject (textures[ID(two_points_id)].get(), real_aspect_ratio));
        GameObject_Handle three_points_object        (new GameObject (textures[ID(three_points_id)].get(), real_aspect_ratio));


        zero_points_object  -> set_position({(canvas_width * 0.5f), (canvas_height * 0.5f)});
        one_points_object   -> set_position({(canvas_width * 0.5f), (canvas_height * 0.5f)});
        two_points_object   -> set_position({(canvas_width * 0.5f), (canvas_height * 0.5f)});
        three_points_object -> set_position({(canvas_width * 0.5f), (canvas_height * 0.5f)});

        play_button_object  -> set_position({(canvas_width * 0.5f), (zero_points_object -> get_bottom_y() - play_button_object  -> get_height())});
        salir_button_object  -> set_position({(canvas_width * 0.5f),(play_button_object -> get_bottom_y() - salir_button_object -> get_height())});

        buttons.push_back(play_button_object);
        buttons.push_back(salir_button_object);
        buttons.push_back(zero_points_object);
        buttons.push_back(one_points_object);
        buttons.push_back(two_points_object);
        buttons.push_back(three_points_object);



        // Se guardan punteros a los gameobjects que se van a usar frecuentemente:
        // nombre_puntero = nombre_objeto.get();

        play_button_pointer  = play_button_object .get();
        salir_button_pointer = salir_button_object.get();
        zero_points_pointer  = zero_points_object .get();
        one_points_pointer   = one_points_object  .get();
        two_points_pointer   = two_points_object  .get();
        three_points_pointer = three_points_object.get();

        //Hacemos invisibles los marcadores hasta conocer la puntuación del usuario
        zero_points_pointer  -> hide();
        one_points_pointer   -> hide();
        two_points_pointer   -> hide();
        three_points_pointer -> hide();

    }

    // ---------------------------------------------------------------------------------------------
    void Final_Scene::run_simulation (float time)
    {
        // Se actualiza el estado de todos los gameobjects:

        for (auto & button : buttons)
        {
            button->update (time);
        }


            switch (points)
            {
                case 0: zero_points_pointer -> show(); break;
                case 1: one_points_pointer -> show(); break;
                case 2: two_points_pointer -> show(); break;
                case 3: three_points_pointer -> show(); break;
            }


    }

    // ---------------------------------------------------------------------------------------------
    // Simplemente se dibujan todos los gameobjects que conforman la escena.

    void Final_Scene::render_playfield (Canvas & canvas)
    {
        for (auto & button : buttons)
        {
            button->render (canvas);
        }
    }



    // ---------------------------------------------------------------------------------------------
    // Se inicia la partida

    void Final_Scene::play() {
        director.run_scene (shared_ptr< Scene >(new Game_Scene));
    }



}
