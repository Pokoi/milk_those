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


#include "Menu_Scene.hpp"
#include "Game_Scene.hpp"

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
    Menu_Scene::Texture_Data Menu_Scene::textures_data[] =
            {
                    //{ ID(nombre_ID),  "game-scene/nombre_archivo.extension"},
                    { ID(play_button_id),          "menu/jugar.png"},
                    { ID(instructions_button_id),  "menu/instrucciones.png"},
                    { ID(logo_button_id),          "menu/milk_those_logo.png"},
                    { ID(instructions_text_id),    "menu/intrucciones-texto.png"},

                    //...

            };

    // Para determinar el número de items en el array textures_data, se divide el tamaño en bytes
    // del array completo entre el tamaño en bytes de un item:

    unsigned Menu_Scene::textures_count = sizeof(textures_data) / sizeof(Texture_Data);

    Menu_Scene::Menu_Scene()
    {
        state         = LOADING;
        suspended     = true;
        canvas_width  = 720;
        canvas_height = 1280;

        aspect_ratio_adjusted = false;
    }

    // ---------------------------------------------------------------------------------------------

    bool Menu_Scene::initialize ()
    {
        state     = LOADING;
        suspended = true;
        showing_instructions = false;

        return true;
    }

    // ---------------------------------------------------------------------------------------------

    void Menu_Scene::handle (basics::Event & event)
    {
        if (state == READY)                     // Se descartan los eventos cuando la escena está LOADING
        {
            switch (event.id)
            {
                case ID(touch-started):         // El usuario toca la pantalla
                {
                    Point2f touch_location = { *event[ID(x)].as< var::Float > (), *event[ID(y)].as< var::Float > () };


                    if(!showing_instructions)
                    {

                        if      (play_button_pointer        ->contains(touch_location)) play();
                        else if (instructions_button_pointer->contains(touch_location)) show_instructions(true);

                    }

                    else if(showing_instructions)
                    {
                        show_instructions(false);

                    }

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

    void Menu_Scene::update (float time)
    {

        if (!suspended) switch (state)
            {
                case LOADING: load_textures  ();     break;
                case READY: run_simulation (time); break;
                case ERROR:   break;
            }

    }

    // ---------------------------------------------------------------------------------------------

    void Menu_Scene::render (Graphics_Context::Accessor & context)
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

    void Menu_Scene::load_textures ()
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
        else {
            //if (timer.get_elapsed_seconds () > 1.f)         // Si las texturas se han cargado muy rápido
            //{                                               // se espera un segundo desde el inicio de
            create_gameobjects();                       // la carga antes de pasar al juego para que
            state = READY;                              // el mensaje de carga no aparezca y desaparezca
            // demasiado rápido.

            //}
        }
    }


    // ---------------------------------------------------------------------------------------------
    // Ajusta el aspect ratio

    void Menu_Scene::adjust_aspect_ratio(Context & context)
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

    void Menu_Scene::create_gameobjects()
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
        GameObject_Handle instructions_button_object (new GameObject (textures[ID(instructions_button_id)]. get(), real_aspect_ratio));
        GameObject_Handle logo_object                (new GameObject (textures[ID(logo_button_id)].get(), real_aspect_ratio));
        GameObject_Handle instructions_text_object   (new GameObject (textures[ID(instructions_text_id)].get(), real_aspect_ratio));

        logo_object        -> set_position({(canvas_width * 0.5f), (canvas_height - (logo_object  -> get_height() * 0.5f))});
        play_button_object -> set_position({(canvas_width * 0.5f), (canvas_height * 0.5f)});
        instructions_button_object        -> set_position({(canvas_width * 0.5f), ((play_button_object -> get_bottom_y()) - (instructions_button_object -> get_height() * 0.5f))});
        instructions_text_object          -> set_position({(canvas_width * 0.5f), (canvas_height * 0.5f)});

        buttons.push_back(play_button_object);
        buttons.push_back(logo_object);
        buttons.push_back(instructions_button_object);
        buttons.push_back(instructions_text_object);




        // Se guardan punteros a los gameobjects que se van a usar frecuentemente:
        // nombre_puntero = nombre_objeto.get();

        play_button_pointer         = play_button_object .get();
        instructions_button_pointer = instructions_button_object.get();
        logo_pointer                = logo_object.get();
        instructions_text_pointer   = instructions_text_object.get();

        instructions_text_pointer -> hide();

    }

    // ---------------------------------------------------------------------------------------------
    void Menu_Scene::run_simulation (float time)
    {
        // Se actualiza el estado de todos los gameobjects:

        for (auto & button : buttons)
        {
            button->update (time);
        }


    }

    // ---------------------------------------------------------------------------------------------
    // Simplemente se dibujan todos los gameobjects que conforman la escena.

    void Menu_Scene::render_playfield (Canvas & canvas)
    {
        for (auto & button : buttons)
        {
            button->render (canvas);
        }
    }



    // ---------------------------------------------------------------------------------------------
    // Se inicia la partida

    void Menu_Scene::play() {
        director.run_scene (shared_ptr< Scene >(new Game_Scene));
    }

    // ---------------------------------------------------------------------------------------------
    // Se muestran las instrucciones

    void Menu_Scene::show_instructions(bool _showing) {

        if(!_showing)
        {
            showing_instructions = false;
            instructions_text_pointer   -> hide();
            logo_pointer                -> show();
            instructions_button_pointer -> show();
            play_button_pointer         -> show();
        }
        else if(_showing)
        {
            showing_instructions = true;

            instructions_text_pointer   -> show();
            logo_pointer                -> hide();
            instructions_button_pointer -> hide();
            play_button_pointer         -> hide();

        }



    }

}
