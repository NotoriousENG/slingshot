package io.github.notoriouseng.slingshot

import com.badlogic.gdx.ApplicationAdapter
import com.badlogic.gdx.Gdx
import com.badlogic.gdx.graphics.Texture
import com.badlogic.gdx.graphics.g2d.SpriteBatch
import com.badlogic.gdx.graphics.glutils.ShapeRenderer
import com.badlogic.gdx.graphics.Color
import com.badlogic.gdx.math.Vector2
import com.badlogic.gdx.math.Vector
import com.badlogic.gdx.Input.Keys
import com.badlogic.gdx.Input
import ktx.app.clearScreen

data class Shot(var pos: Vector2, var vel: Vector2)

/** [com.badlogic.gdx.ApplicationListener] implementation shared by all platforms. */
class Main : ApplicationAdapter() {
    private lateinit var spriteBatch: SpriteBatch
    private lateinit var shapeRenderer: ShapeRenderer
    private lateinit var texSlingshot: Texture
    private lateinit var texBall: Texture
    private var position: Vector2 = Vector2(350.0f, 64.0f)
    private val speed = 300.0f
    private val shots = arrayListOf<Shot>()

    override fun create() {
        spriteBatch = SpriteBatch()
        shapeRenderer = ShapeRenderer()
        texSlingshot = Texture(Gdx.files.internal("textures/slingshot.png"))
        texBall = Texture(Gdx.files.internal("textures/ball.png"))
    }

    override fun render() {
        val delta = Gdx.graphics.deltaTime
        update(delta)
        draw()
    }

    private fun update(delta: Float) {
        val screenHeight = Gdx.graphics.getHeight().toFloat();

        val mouseX = Gdx.input.x.toFloat()
        val mouseY = screenHeight - Gdx.input.y.toFloat()
        this.position.x = mouseX - 16.0f
        this.position.y = mouseY - 16.0f
        this.position.x = Math.clamp(this.position.x, -16.0f, 816.0f)
        this.position.y = Math.clamp(this.position.y, -16.0f, 616.0f)

        if (Gdx.input.isButtonJustPressed(Input.Buttons.LEFT)) {
            val origin = Vector2(348.0f, 100.0f);
            val dir = origin.cpy().sub(this.position).nor()
            val fac = Math.abs(origin.cpy().dst(this.position)) * 10.0f
            val shot = Shot(this.position.cpy(), dir.cpy().scl(fac))
            this.shots.add(shot)
        }

        updateShots(delta)
    }

    private fun updateShots(delta: Float) {
        val screenWidth = Gdx.graphics.getWidth().toFloat();
        val screenHeight = Gdx.graphics.getHeight().toFloat();
        val iterator = shots.iterator()
        val gravity = -980f
        while (iterator.hasNext()) {
            val shot = iterator.next()
            // Apply gravity to the vertical direction
            shot.vel.y += gravity * delta
            shot.pos.add(shot.vel.cpy().scl(delta))
            val dim = 32.0f;
            if (shot.pos.x > screenWidth + dim || shot.pos.x < -dim || shot.pos.y > screenHeight + dim || shot.pos.y < -dim) {
                iterator.remove()
            }
        }
    }

    private fun draw() {
        clearScreen(0f, 0f, 0f, 1f)
        spriteBatch.begin()
        spriteBatch.draw(texSlingshot, 300.0f, 000.0f)
        spriteBatch.draw(texBall, this.position.x, this.position.y)
        for (shot in this.shots) {
            spriteBatch.draw(texBall, shot.pos.x, shot.pos.y)
        }
        spriteBatch.end()

        // slingshot strings
        shapeRenderer.begin(ShapeRenderer.ShapeType.Line)
        shapeRenderer.color = Color.WHITE
        shapeRenderer.line(310.0f, 100.0f, this.texBall.width/2 + this.position.x, this.texBall.height/2 +this.position.y)
        shapeRenderer.line(418.0f, 100.0f, this.texBall.width/2 + this.position.x, this.texBall.height/2 +this.position.y)
        shapeRenderer.end()
    }

    override fun dispose() {
        spriteBatch.dispose()
        shapeRenderer.dispose()
        texSlingshot.dispose()
        texBall.dispose()
    }
}
